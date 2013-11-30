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

#include <unordered_map>

class GameServer;

static void onTTYAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
static void onTTYRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

static void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string &resultString);
static void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
static void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);
static void onAuthClosed(IListener* instance, Authentication* auth);
static void onAuthClosedWithFailure(IListener* instance, Authentication* auth);

static void onGamePacketReceived(IListener* instance, RappelzSocket* socket, const TS_MESSAGE* packet);
static void onSocketStateChange(IListener* instance, Socket* socket, Socket::State oldState, Socket::State newState);
static void onSocketTimer(uv_timer_t* handle);
static void onSocketGGTimer(uv_timer_t* handle);
static void onSocketUpdateTimer(uv_timer_t* handle);

static void resendMsg(GameServer* gameServer, int type, const char* sender, std::string msg);


bool printDebug = false;
bool usersa;
std::string ip;
int port;
bool enableGateway = false;

class GameServer : public IListener {
public:
	GameServer(const std::string& ip, int port, std::string account, std::string password, int serverIdx, std::string playername, Log* packetLog)
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
		uv_timer_start(&updateTimer, &onSocketUpdateTimer, 5000, 5000);
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

std::vector<GameServer*> gameServers;
uv_tty_t ttyHandle;
char ttyBuffer[1000];

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
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4500);

	CFG_CREATE("account1", "test1");
	CFG_CREATE("password1", "admin");
	CFG_CREATE("gsindex1", 1);
	CFG_CREATE("charname1", "Player1");

	CFG_CREATE("account2", "test2");
	CFG_CREATE("password2", "admin");
	CFG_CREATE("gsindex2", 2);
	CFG_CREATE("charname2", "Player2");

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
	ip = CFG_GET("ip")->getString();
	port = CFG_GET("port")->getInt();
	enableGateway = CFG_GET("gateway")->getBool();

	std::string account1 = CFG_GET("account1")->getString();
	std::string password1 = CFG_GET("password1")->getString();
	int serverIdx1 = CFG_GET("gsindex1")->getInt();
	std::string charname1 = CFG_GET("charname1")->getString();

	std::string account2 = CFG_GET("account2")->getString();
	std::string password2 = CFG_GET("password2")->getString();
	int serverIdx2 = CFG_GET("gsindex2")->getInt();
	std::string charname2 = CFG_GET("charname2")->getString();


	fprintf(stderr, "Starting chat gateway\n");

	gameServers.push_back(new GameServer(ip, port, account1, password1, serverIdx1, charname1, &trafficLogger));
	gameServers.push_back(new GameServer(ip, port, account2, password2, serverIdx2, charname2, &trafficLogger));

	uv_tty_init(EventLoop::getLoop(), &ttyHandle, 0, 1);
	uv_read_start((uv_stream_t*)&ttyHandle, &onTTYAlloc, &onTTYRead);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

static void onTTYAlloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	memset(ttyBuffer, 0, sizeof(ttyBuffer));
	buf->base = ttyBuffer;
	buf->len = sizeof(ttyBuffer);
}

static void onTTYRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
	int gsIndex = 0;
	int type = TS_CS_CHAT_REQUEST::CHAT_NORMAL;
	std::string target = "";
	std::string message;

	printf("tty: %s\n", ttyBuffer);
	char *p = ttyBuffer;
	if(p[0] >= '0' && p[0] <= '9') {
		gsIndex = p[0] - '0';
		p++;
	}

	switch(p[0]) {
		case '\"':
			type = TS_CS_CHAT_REQUEST::CHAT_WHISPER;
			p++;
			break;
	}
	std::string inputBuffer(p);
	if(type == TS_CS_CHAT_REQUEST::CHAT_WHISPER) {
		size_t pos = inputBuffer.find_first_of(' ');
		target = inputBuffer.substr(0, pos);
		if(pos == std::string::npos)
			message = inputBuffer;
		else
			message = inputBuffer.substr(pos+1);
	} else
		message = inputBuffer;

	char messageFull[500];
	sprintf(messageFull, "[%d] %s: %s", gsIndex, target.c_str(), message.c_str());
	printf("tty Msg %d: %s\n", type, messageFull);

	if(gsIndex < (int)gameServers.size() && gameServers.at(gsIndex)->socket && message.size() > 0) {
		TS_CS_CHAT_REQUEST* chatRqst = TS_MESSAGE_WNA::create<TS_CS_CHAT_REQUEST, char>(message.size());

		chatRqst->len = message.size();
		strcpy(chatRqst->szTarget, target.c_str());
		strncpy(chatRqst->message, message.c_str(), chatRqst->len);
		chatRqst->type = type;
		chatRqst->request_id = 0;

		gameServers.at(gsIndex)->socket->sendPacket(chatRqst);

		TS_MESSAGE_WNA::destroy(chatRqst);
	}
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
		uv_timer_start(&gameServer->recoTimer, &onSocketTimer, 5000, 0);
		printf("%p timer reco\n", instance);
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
	fprintf(stderr, "%p login to GS result: %d\n", instance, result);
	uv_timer_start(&gameServer->ggTimer, &onSocketGGTimer, 280000, 0);

	gameServerSocket->addEventListener(instance, &onSocketStateChange);
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

			break;
		}
		case TS_SC_LOGIN_RESULT::packetID: {
				TS_SC_LOGIN_RESULT* loginResultPkt = (TS_SC_LOGIN_RESULT*) packet;
				gameServer->handle = loginResultPkt->handle;
				printf("%p this char handle: 0x%08X\n", instance, gameServer->handle);
				break;
			}
		case TS_SC_ENTER::packetID: {
			TS_SC_ENTER* enterPkt = (TS_SC_ENTER*) packet;
			if(enterPkt->type == 0 && enterPkt->ObjType == 0) {
				TS_SC_ENTER::PlayerInfo* playerInfo = (TS_SC_ENTER::PlayerInfo*) (((char*)enterPkt) + sizeof(TS_SC_ENTER));
				playerNames.insert(std::pair<unsigned int, std::string>(enterPkt->handle, std::string(playerInfo->szName)));
				printf("%p Added player %s with handle 0x%08X\n", instance, playerInfo->szName, enterPkt->handle);
			}
			break;
		}
		case TS_SC_CHAT_LOCAL::packetID: {
			TS_SC_CHAT_LOCAL* chatPkt = (TS_SC_CHAT_LOCAL*) packet;
			std::string msg = std::string(chatPkt->message, chatPkt->len);
			std::unordered_map<unsigned int, std::string>::iterator it = playerNames.find(chatPkt->handle);
			std::string playerName = "Unknown";

			if(it != playerNames.end())
				playerName = it->second;
			else if(chatPkt->handle == gameServer->handle)
				playerName = gameServer->playername;

			resendMsg(gameServer, chatPkt->type, playerName.c_str(), msg);
			break;
		}
		case TS_SC_CHAT::packetID: {
			TS_SC_CHAT* chatPkt = (TS_SC_CHAT*) packet;
			std::string msg = std::string(chatPkt->message, chatPkt->len);
			resendMsg(gameServer, chatPkt->type, chatPkt->szSender, msg);
			break;
		}

		case TS_SC_DISCONNECT_DESC::packetID:
			if(gameServer->socket) {
				printf("%p disconnect\n", instance);
				gameServer->socket->close();
			} else
				printf("%p can't disconnect no socket\n", instance);
			break;
	}
}

static void resendMsg(GameServer* gameServer, int type, const char* sender, std::string msg) {
	char messageFull[500];
	int msgLen;

	if(!strcmp(sender, gameServer->playername.c_str()))
		return;

	std::replace(msg.begin(), msg.end(), '\x0D', '\x0A');

	sprintf(messageFull, "[%s] %s: %s", gameServer->serverName.c_str(), sender, msg.c_str());
	printf("%p Msg %d: %s\n", gameServer, type, messageFull);

	if(sender[0] == '@')
		return;

	if(type != TS_CS_CHAT_REQUEST::CHAT_NORMAL && type != TS_CS_CHAT_REQUEST::CHAT_WHISPER)
		return;

	if(msg.size() < 1)
		return;

	msgLen = (strlen(messageFull) > 255) ? 255 : strlen(messageFull);

	TS_CS_CHAT_REQUEST* chatRqst;

	if(type == TS_CS_CHAT_REQUEST::CHAT_WHISPER) {
		const char* messageToWhisper = "Ce perso fait uniquement passerelle entre le chat des serveurs Joker et Pyro. En gros je dis ce qui se dit sur l'autre serveur";
		chatRqst = TS_MESSAGE_WNA::create<TS_CS_CHAT_REQUEST, char>(strlen(messageToWhisper));

		chatRqst->len = strlen(messageToWhisper);
		strcpy(chatRqst->szTarget, sender);
		strncpy(chatRqst->message, messageToWhisper, chatRqst->len);
		chatRqst->type = TS_CS_CHAT_REQUEST::CHAT_WHISPER;
		chatRqst->request_id = 0;
	} else {
		chatRqst = TS_MESSAGE_WNA::create<TS_CS_CHAT_REQUEST, char>(msgLen);

		chatRqst->len = msgLen;
		strcpy(chatRqst->szTarget, "");
		strncpy(chatRqst->message, messageFull, chatRqst->len);
		chatRqst->type = TS_CS_CHAT_REQUEST::CHAT_NORMAL;
	}

	if(enableGateway) {
		if(type == TS_CS_CHAT_REQUEST::CHAT_WHISPER) {
			gameServer->socket->sendPacket(chatRqst);
		} else {
			for(auto it = gameServers.begin(); it != gameServers.end(); ++it) {
				GameServer* otherServer = *it;

				if(otherServer != gameServer && otherServer->socket)
					otherServer->socket->sendPacket(chatRqst);
			}
		}
	}

	TS_MESSAGE_WNA::destroy(chatRqst);
}

static void onSocketStateChange(IListener* instance, Socket* socket, Socket::State oldState, Socket::State newState) {
	if(newState == Socket::UnconnectedState) {
		GameServer* gameServer = (GameServer*) instance;
		gameServer->socket = nullptr;
		uv_timer_start(&gameServer->recoTimer, &onSocketTimer, 5000, 0);
		printf("%p timer reco\n", instance);
	}
}

static void onSocketGGTimer(uv_timer_t* handle) {
	GameServer* gameServer = (GameServer*) handle->data;
	if(gameServer->socket) {
		printf("%p gg reco\n", gameServer);
		gameServer->socket->close();
	} else {
		printf("%p can't gg reco no socket !!!\n", gameServer);
	}
}

static void onSocketTimer(uv_timer_t* handle) {
	GameServer* gameServer = (GameServer*) handle->data;
	printf("%p reco\n", gameServer);
	gameServer->connect();
}

static void onSocketUpdateTimer(uv_timer_t* handle) {
	GameServer* gameServer = (GameServer*) handle->data;
	TS_CS_UPDATE updatPkt;
	TS_MESSAGE::initMessage<TS_CS_UPDATE>(&updatPkt);

	updatPkt.handle = gameServer->handle;

	if(!gameServer->socket)
		return;


	gameServer->socket->sendPacket(&updatPkt);
}
