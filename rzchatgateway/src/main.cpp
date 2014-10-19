#include "uv.h"
#include "Account.h"
#include "Authentication.h"
#include <string.h>
#include "EventLoop.h"
#include "RappelzLibInit.h"
#include <stdio.h>
#include "RappelzSocket.h"
#include "ConfigInfo.h"
#include "RappelzLibConfig.h"
#include "Log.h"
#include <algorithm>

#include "Packets/TS_SC_CHAT.h"
#include "Packets/TS_SC_CHAT_LOCAL.h"
#include "Packets/TS_CS_CHAT_REQUEST.h"
#include "Packets/TS_CS_CHARACTER_LIST.h"
#include "Packets/TS_SC_CHARACTER_LIST.h"
#include "Packets/TS_CS_LOGIN.h"
#include "Packets/TS_SC_LOGIN_RESULT.h"
#include "Packets/TS_TIMESYNC.h"
#include "Packets/TS_SC_ENTER.h"
#include "Packets/TS_SC_DISCONNECT_DESC.h"
#include "Packets/TS_CS_UPDATE.h"

#include <vector>
#include <string>
#include <unordered_map>

class GameServer;

static void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string &resultString);
static void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
static void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);
static void onAuthClosed(IListener* instance, Authentication* auth);
static void onAuthClosedWithFailure(IListener* instance, Authentication* auth);

static void onGamePacketReceived(IListener* instance, RappelzSocket* socket, const TS_MESSAGE* packet);
static void onGSSocketStateChange(IListener* instance, Socket* socket, Socket::State oldState, Socket::State newState);
static void onGSSocketTimer(uv_timer_t* handle);
static void onGSSocketGGTimer(uv_timer_t* handle);
static void onGSSocketUpdateTimer(uv_timer_t* handle);

static void sendMsgToGS(int type, const char* sender, const char* target, std::string msg);
static void sendMsgToIRC(int type, const char* sender, std::string msg);


bool printDebug = false;
bool usersa;
std::string ip;
int port;
bool enableGateway = false;

std::unordered_map<std::string, std::string> mpRepliesPlayers;

class GameServer : public IListener {
public:
	GameServer(const std::string& ip, int port, const std::string& account, const std::string& password, int serverIdx, const std::string& playername, Log* packetLog)
		: auth(new Authentication(ip, usersa? Authentication::ACM_RSA_AES : Authentication::ACM_DES, port, packetLog)),
		  serverIdx(serverIdx),
		  password(password),
		  playername(playername)
	{
		this->account = new Account(account);
		socket = nullptr;
		handle = 0;
		uv_timer_init(EventLoop::getLoop(), &ggTimer);
		uv_timer_init(EventLoop::getLoop(), &recoTimer);
		uv_timer_init(EventLoop::getLoop(), &updateTimer);
		ggTimer.data = this;
		recoTimer.data = this;
		updateTimer.data = this;
		uv_timer_start(&updateTimer, &onGSSocketUpdateTimer, 5000, 5000);
		connect();
	}

	void connect() {
		auth->connect(this->account, password, Callback<Authentication::CallbackOnAuthResult>(this, &onAuthResult));
	}

	~GameServer() {
		delete auth;
		delete account;
	}

	Authentication* auth;
	Account* account;
	int serverIdx;
	std::string password;
	std::string playername;
	std::string serverName;
	RappelzSocket* socket;
	uv_timer_t ggTimer;
	uv_timer_t recoTimer;
	uv_timer_t updateTimer;

	unsigned int handle;

	std::list<std::string> messageQueue;
};

class IrcClient : public IListener {
public:
	IrcClient(const std::string& ip, int port, const std::string& channel, const std::string& nickname, Log* packetLog)
		: channelName(channel), nickname(nickname), ip(ip), port(port), socket(EventLoop::getLoop())
	{
		socket.addEventListener(this, &onSocketStateChange);
		socket.addDataListener(this, &onSocketDataStatic);
		socket.setPacketLogger(packetLog);
	}

	void connect(std::string servername) {
		this->servername = servername.substr(0, servername.find_first_of('('));
		socket.connect(ip, port);
	}

	static void onSocketStateChange(IListener* instance, Socket* socket, Socket::State oldState, Socket::State newState) {
		IrcClient* thisInstance = (IrcClient*) instance;
		if(newState == Socket::ConnectedState) {
			char loginText[128];
			std::string lowerCaseName;

			if(thisInstance->nickname.size() == 0)
				thisInstance->nickname = thisInstance->servername;

			lowerCaseName = thisInstance->nickname;
			lowerCaseName[0] = tolower(lowerCaseName[0]);

			if(thisInstance->channelName.size() == 0)
				thisInstance->channelName = "#rappelz-" + lowerCaseName;

			sprintf(loginText, "NICK %s\r\nUSER gw-%s * * : In-game chat relay with rappelz server %s\r\nJOIN %s\r\n",
					thisInstance->nickname.c_str(),
					lowerCaseName.c_str(),
					thisInstance->servername.c_str(),
					thisInstance->channelName.c_str());

			thisInstance->socket.write(loginText, strlen(loginText));
		}
	}

	static void onSocketDataStatic(IListener* instance, Socket* socket) { ((IrcClient*) instance)->onSocketData(socket); }
	void onSocketData(Socket* socket) {
		std::vector<char> dataRecv;
		socket->readAll(&dataRecv);
		char *p;

		buffer.insert(buffer.end(), dataRecv.begin(), dataRecv.end());
		while(buffer.size() > 0 && (p = (char*)Utils::memmem(&buffer[0], buffer.size(), "\r\n", 2))) {
			std::string line(&buffer[0], p);
			buffer.erase(buffer.begin(), buffer.begin() + (p - &buffer[0]) + 2);
			onIrcLine(line);
		}
	}

	static void parseIrcMessage(const std::string& message, std::string& prefix, std::string& command, std::string& parameters, std::string& trailing)
	{
		size_t prefixEnd, trailingStart;
		size_t commandEnd;

		// Grab the prefix if it is present. If a message begins
		// with a colon, the characters following the colon until
		// the first space are the prefix.
		if (message[0] == ':') {
			prefixEnd = message.find_first_of(' ');
			prefix.assign(message, 1, prefixEnd - 1);
			prefixEnd++;
		} else {
			prefixEnd = 0;
		}

		// Grab the trailing if it is present. If a message contains
		// a space immediately following a colon, all characters after
		// the colon are the trailing part.
		trailingStart = message.find(" :");
		if (trailingStart != std::string::npos)
			trailing.assign(message, trailingStart+2, std::string::npos);
		else
			trailingStart = message.size();

		commandEnd = message.find_first_of(" \r\n", prefixEnd);
		if(commandEnd == std::string::npos)
			commandEnd = message.size();

		command.assign(message, prefixEnd, commandEnd - prefixEnd);
		parameters.assign(message, commandEnd, trailingStart - commandEnd);
	}

	void onIrcLine(const std::string& line) {
		std::string prefix;
		std::string command;
		std::string parameters;
		std::string trailing;

		parseIrcMessage(line, prefix, command, parameters, trailing);

		if (command == "PING") {
			std::string pong = "PONG :" + trailing;
			socket.write(pong.c_str(), pong.size());
		} else if(command == "PRIVMSG") {
			std::string sender;

			if(trailing.size() == 0)
				return;

			sender.assign(prefix, 0, prefix.find_first_of('!'));

			switch(trailing[0]) {
				case '\"': {
					std::string target, msg;
					size_t separator = trailing.find_first_of(' ');

					target.assign(trailing, 1, separator-1);

					if(separator == std::string::npos || separator == trailing.size() - 1) {
						sendMessage(sender.c_str(), "Utilisation des messages priv\xE9s comme en jeu: \"Player message \xE0 envoyer");
						break;
					}

					msg.assign(trailing, separator+1, std::string::npos);
					mpRepliesPlayers.insert(std::pair<std::string, std::string>(target, sender));
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_WHISPER, sender.c_str(), target.c_str(), msg.c_str());
					break;
				}

				case '$':
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_ADV, sender.c_str(), "", trailing.c_str());
					break;

				case '!':
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_GLOBAL, sender.c_str(), "", trailing.c_str());
					break;

				case '#':
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_PARTY, sender.c_str(), "", trailing.c_str());
					break;

				case '%':
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_GUILD, sender.c_str(), "", trailing.c_str());
					break;

				default:
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_NORMAL, sender.c_str(), "", trailing.c_str());
			}
		}
	}

	void sendMessage(const char* target, const std::string& msg) {
		char msgText[512];
		if(target && (target[0] == '\0' || target[0] == '#'))
			sprintf(msgText, "PRIVMSG %s :%s\r\n", channelName.c_str(), msg.c_str());
		else
			sprintf(msgText, "PRIVMSG %s :%s\r\n", target, msg.c_str());
		socket.write(msgText, strlen(msgText));
	}

	std::string servername;
	std::string nickname;
	std::string channelName;
	std::string ip;
	int port;

	Socket socket;
	std::vector<char> buffer;
};

GameServer* gameServer = nullptr;
IrcClient* ircClient = nullptr;

struct TrafficDump {
	cval<bool> &enable;
	cval<std::string> &dir, &file, &level, &consoleLevel;

	TrafficDump() :
		enable(CFG_CREATE("trafficdump.enable", false)),
		dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		file(CFG_CREATE("trafficdump.file", "chatgateway.log")),
		level(CFG_CREATE("trafficdump.level", "debug")),
		consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal"))
	{
		Utils::autoSetAbsoluteDir(dir);
	}
}* trafficDump;

static void init() {
	CFG_CREATE("rappelz.ip", "127.0.0.1");
	CFG_CREATE("rappelz.port", 4500);

	CFG_CREATE("rappelz.account", "account");
	CFG_CREATE("rappelz.password", "password");
	CFG_CREATE("rappelz.gsindex", 2);
	CFG_CREATE("rappelz.charname", "Yrolis");

	CFG_CREATE("irc.ip", "127.0.0.1");
	CFG_CREATE("irc.port", 6667);
	CFG_CREATE("irc.channel", "");
	CFG_CREATE("irc.nick", "");

	CFG_CREATE("use_rsa", true);
	CFG_CREATE("printall", false);
	CFG_CREATE("gateway", false);

	trafficDump = new TrafficDump;

	RappelzLibConfig::get()->app.appName.setDefault("RappelzChatGateway");
	RappelzLibConfig::get()->app.configfile.setDefault("chatgateway.opt");
	RappelzLibConfig::get()->log.file.setDefault("chatgateway.log");
}


int main(int argc, char *argv[])
{
	RappelzLibInit();
	init();
	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(RappelzLibConfig::get()->log.enable,
				   RappelzLibConfig::get()->log.level,
				   RappelzLibConfig::get()->log.consoleLevel,
				   RappelzLibConfig::get()->log.dir,
				   RappelzLibConfig::get()->log.file,
				   RappelzLibConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	Log trafficLogger(trafficDump->enable,
					  trafficDump->level,
					  trafficDump->consoleLevel,
					  trafficDump->dir,
					  trafficDump->file,
					  RappelzLibConfig::get()->log.maxQueueSize);

	ConfigInfo::get()->dump();

	printDebug = CFG_GET("printall")->getBool();
	usersa = CFG_GET("use_rsa")->getBool();
	ip = CFG_GET("rappelz.ip")->getString();
	port = CFG_GET("rappelz.port")->getInt();
	enableGateway = CFG_GET("gateway")->getBool();

	std::string ircIp = CFG_GET("irc.ip")->getString();
	int ircPort = CFG_GET("irc.port")->getInt();
	std::string ircNick = CFG_GET("irc.nick")->getString();
	std::string ircChannel = CFG_GET("irc.channel")->getString();

	std::string account = CFG_GET("account")->getString();
	std::string password = CFG_GET("password")->getString();
	int serverIdx = CFG_GET("gsindex")->getInt();
	std::string charname = CFG_GET("charname")->getString();

	mainLogger.info("Starting chat gateway\n");

	gameServer = new GameServer(ip, port, account, password, serverIdx, charname, &trafficLogger);
	ircClient = new IrcClient(ircIp, ircPort, ircChannel, ircNick, &trafficLogger);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

//auth bench
static void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string& resultString) {
	GameServer* gameServer = (GameServer*) instance;
	if(printDebug || result != TS_RESULT_SUCCESS)
		fprintf(stderr, "%p %s: Auth result: %d (%s)\n", instance, auth->getAccountName().c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
	if(result == TS_RESULT_SUCCESS) {
		auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(instance, &onServerList));
	} else if(result == TS_RESULT_ALREADY_EXIST) {
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(instance, &onAuthClosedWithFailure));
		uv_timer_start(&gameServer->recoTimer, &onGSSocketTimer, 5000, 0);
	} else {
	}
}

static void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId) {
	GameServer* gameServer = (GameServer*) instance;
	if(printDebug) {
		fprintf(stderr, "%p Server list (last id: %d)\n", instance, lastSelectedServerId);
		for(size_t i = 0; i < servers->size(); i++) {
			fprintf(stderr, "%ld %d: %20s at %16s:%d %d%% user ratio\n",
					(long)i,
					servers->at(i).serverId,
					servers->at(i).serverName.c_str(),
					servers->at(i).serverIp.c_str(),
					servers->at(i).serverPort,
					servers->at(i).userRatio);
		}
	}
	if((int)servers->size() > gameServer->serverIdx) {
		gameServer->serverName = servers->at(gameServer->serverIdx).serverName;
		if(ircClient->socket.getState() != Socket::ConnectedState)
			ircClient->connect(gameServer->serverName);
		auth->selectServer(servers->at(gameServer->serverIdx).serverId, Callback<Authentication::CallbackOnGameResult>(instance, &onGameResult));
	} else {
		fprintf(stderr, "%p server not available\n", instance);
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(instance, &onAuthClosed));
	}
}

static void onAuthClosedWithFailure(IListener* instance, Authentication* auth) {
	if(printDebug)
		fprintf(stderr, "%p auth closed with failure\n", instance);
}

static void onAuthClosed(IListener* instance, Authentication* auth) {
	if(printDebug)
		fprintf(stderr, "%p auth closed\n", instance);
}

static void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket) {
	GameServer* gameServer = (GameServer*) instance;
	if(printDebug)
		fprintf(stderr, "%p login to GS result: %d\n", instance, result);
	uv_timer_start(&gameServer->ggTimer, &onGSSocketGGTimer, 280000, 0);

	gameServerSocket->addEventListener(instance, &onGSSocketStateChange);
	gameServerSocket->addPacketListener(TS_SC_CHAT::packetID, instance, &onGamePacketReceived);
	gameServerSocket->addPacketListener(TS_SC_CHARACTER_LIST::packetID, instance, &onGamePacketReceived);
	gameServerSocket->addPacketListener(TS_SC_ENTER::packetID, instance, &onGamePacketReceived);
	gameServerSocket->addPacketListener(TS_SC_CHAT_LOCAL::packetID, instance, &onGamePacketReceived);
	gameServerSocket->addPacketListener(TS_SC_DISCONNECT_DESC::packetID, instance, &onGamePacketReceived);
	gameServerSocket->addPacketListener(TS_SC_LOGIN_RESULT::packetID, instance, &onGamePacketReceived);

	TS_CS_CHARACTER_LIST charlistPkt;
	TS_MESSAGE::initMessage<TS_CS_CHARACTER_LIST>(&charlistPkt);
	strcpy(charlistPkt.account, gameServer->auth->getAccountName().c_str());
	gameServerSocket->sendPacket(&charlistPkt);
}

std::unordered_map<unsigned int, std::string> playerNames;

static void onGamePacketReceived(IListener* instance, RappelzSocket* socket, const TS_MESSAGE* packet) {
	GameServer* gameServer = (GameServer*) instance;
	switch(packet->id) {
		case TS_SC_CHARACTER_LIST::packetID: {
			TS_CS_LOGIN loginPkt;
			TS_TIMESYNC timeSyncPkt;

			TS_MESSAGE::initMessage<TS_CS_LOGIN>(&loginPkt);
			TS_MESSAGE::initMessage<TS_TIMESYNC>(&timeSyncPkt);

			strcpy(loginPkt.szName, gameServer->playername.c_str());
			loginPkt.race = 0;
			socket->sendPacket(&loginPkt);

			timeSyncPkt.time = 0;
			socket->sendPacket(&timeSyncPkt);

			gameServer->socket = socket;
			ircClient->sendMessage("", "\001ACTION is connected to the game server\001");

			break;
		}
		case TS_SC_LOGIN_RESULT::packetID: {
				TS_SC_LOGIN_RESULT* loginResultPkt = (TS_SC_LOGIN_RESULT*) packet;
				gameServer->handle = loginResultPkt->handle;
				break;
			}
		case TS_SC_ENTER::packetID: {
			TS_SC_ENTER* enterPkt = (TS_SC_ENTER*) packet;
			if(enterPkt->type == 0 && enterPkt->ObjType == 0) {
				TS_SC_ENTER::PlayerInfo* playerInfo = (TS_SC_ENTER::PlayerInfo*) (((char*)enterPkt) + sizeof(TS_SC_ENTER));
				playerNames.insert(std::pair<unsigned int, std::string>(enterPkt->handle, std::string(playerInfo->szName)));
			}
			break;
		}
		case TS_SC_CHAT_LOCAL::packetID: {
			TS_SC_CHAT_LOCAL* chatPkt = (TS_SC_CHAT_LOCAL*) packet;
			std::string msg = std::string(chatPkt->message, chatPkt->len);
			std::unordered_map<unsigned int, std::string>::iterator it = playerNames.find(chatPkt->handle);
			std::string playerName = "Unknown";

			if(chatPkt->handle == gameServer->handle)
				break;

			if(it != playerNames.end())
				playerName = it->second;

			sendMsgToIRC(chatPkt->type, playerName.c_str(), msg);
			break;
		}
		case TS_SC_CHAT::packetID: {
			TS_SC_CHAT* chatPkt = (TS_SC_CHAT*) packet;
			std::string msg = std::string(chatPkt->message, chatPkt->len);
			sendMsgToIRC(chatPkt->type, chatPkt->szSender, msg);
			break;
		}

		case TS_SC_DISCONNECT_DESC::packetID:
			if(gameServer->socket) {
				gameServer->socket->close();
			} else
				printf("%p can't disconnect no socket\n", instance);
			break;
	}
}

static void sendMsgToGS(int type, const char* sender, const char* target, std::string msg) {
	char messageFull[500];
	int msgLen;

	std::replace(msg.begin(), msg.end(), '\x0D', '\x0A');
	if(msg.size() > 200)
		msg.resize(200, ' ');

	if(sender && sender[0])
		sprintf(messageFull, "%s: %s", sender, msg.c_str());
	else
		sprintf(messageFull, "%s", msg.c_str());

	if(printDebug)
		printf("[IRC] Msg %d: %s\n", type, messageFull);

	if(sender[0] == '@')
		return;

	if(msg.size() < 1)
		return;

	msgLen = (strlen(messageFull) > 255) ? 255 : strlen(messageFull);

	TS_CS_CHAT_REQUEST* chatRqst;
	chatRqst = TS_MESSAGE_WNA::create<TS_CS_CHAT_REQUEST, char>(msgLen);

	chatRqst->len = msgLen;
	strcpy(chatRqst->szTarget, target);
	strncpy(chatRqst->message, messageFull, chatRqst->len);
	chatRqst->type = type;
	chatRqst->request_id = 0;

	if(enableGateway) {
		if(gameServer->socket)
			gameServer->socket->sendPacket(chatRqst);
	}

	TS_MESSAGE_WNA::destroy(chatRqst);
}

static const char* getChatColor(int type) {
	switch(type) {
		case TS_CS_CHAT_REQUEST::CHAT_ADV: return "02";
		case TS_CS_CHAT_REQUEST::CHAT_WHISPER: return "08";
		case TS_CS_CHAT_REQUEST::CHAT_GLOBAL: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_EMOTION: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_GM: return "07";
		case TS_CS_CHAT_REQUEST::CHAT_GM_WHISPER: return "08";
		case TS_CS_CHAT_REQUEST::CHAT_PARTY: return "03";
		case TS_CS_CHAT_REQUEST::CHAT_GUILD: return "06";
		case TS_CS_CHAT_REQUEST::CHAT_ATTACKTEAM: return "06";
		case TS_CS_CHAT_REQUEST::CHAT_NOTICE: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_ANNOUNCE: return "15";
		case TS_CS_CHAT_REQUEST::CHAT_CENTER_NOTICE: return "15";
	}
	return nullptr;
}

static void sendMsgToIRC(int type, const char* sender, std::string msg) {
	char messageFull[500];

	std::replace(msg.begin(), msg.end(), '\x0D', ' ');
	std::replace(msg.begin(), msg.end(), '\x0A', ' ');

	const char* color = getChatColor(type);

	if(printDebug)
		printf("[GS] Msg %d: %s: %s\n", type, sender, msg.c_str());

	if(sender[0] == '@')
		return;

	if(msg.size() < 1)
		return;

	if(ircClient) {
		if(type == TS_CS_CHAT_REQUEST::CHAT_WHISPER) {
			size_t separator = msg.find(": ");
			std::string target;

			if(separator == std::string::npos || separator == msg.size()-2) {
				auto it = mpRepliesPlayers.find(std::string(sender));
				if(it == mpRepliesPlayers.end()) {
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_WHISPER, "", sender, "Pour envoyer un message priv\xE9, il faut indiquer le nom suivi de \": \". Exemple:");
					sendMsgToGS(TS_CS_CHAT_REQUEST::CHAT_WHISPER, "", sender, "Player: message \xE0 envoyer");
				} else {
					target = it->second;
					separator = 0;
				}
			} else {
				target.assign(msg, 0, separator);
				separator += 2;
			}

			if(target.size() > 0) {
				if(color)
					sprintf(messageFull, "%c%s%s: %s%c", 0x03, color, sender, msg.substr(separator).c_str(), 0x03);
				else
					sprintf(messageFull, "%s: %s", sender, msg.substr(separator).c_str());

				ircClient->sendMessage(target.c_str(), messageFull);
			}
		} else {
			if(color)
				sprintf(messageFull, "%c%s%s: %s%c", 0x03, color, sender, msg.c_str(), 0x03);
			else
				sprintf(messageFull, "%s: %s", sender, msg.c_str());
			ircClient->sendMessage("", messageFull);
		}
	}
}

static void onGSSocketStateChange(IListener* instance, Socket* socket, Socket::State oldState, Socket::State newState) {
	if(newState == Socket::UnconnectedState) {
		GameServer* gameServer = (GameServer*) instance;
		gameServer->socket = nullptr;
		ircClient->sendMessage("", "\001ACTION disconnected from the game server\001");
		uv_timer_start(&gameServer->recoTimer, &onGSSocketTimer, 5000, 0);
		if(printDebug)
			printf("%p timer reco\n", instance);
	}
}

static void onGSSocketGGTimer(uv_timer_t* handle) {
	GameServer* gameServer = (GameServer*) handle->data;
	if(gameServer->socket) {
		if(printDebug)
			printf("%p gg reco\n", gameServer);
		gameServer->socket->close();
	} else {
		printf("%p can't gg reco no socket !!!\n", gameServer);
	}
}

static void onGSSocketTimer(uv_timer_t* handle) {
	GameServer* gameServer = (GameServer*) handle->data;
	if(printDebug)
		printf("%p reco\n", gameServer);
	gameServer->connect();
}

static void onGSSocketUpdateTimer(uv_timer_t* handle) {
	GameServer* gameServer = (GameServer*) handle->data;
	TS_CS_UPDATE updatPkt;
	TS_MESSAGE::initMessage<TS_CS_UPDATE>(&updatPkt);

	updatPkt.handle = gameServer->handle;

	if(!gameServer->socket)
		return;

	gameServer->socket->sendPacket(&updatPkt);
}
