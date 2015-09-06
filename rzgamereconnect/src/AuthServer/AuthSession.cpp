#include "AuthSession.h"
#include "GameServerSession.h"
#include "../GlobalConfig.h"
#include "Core/EventLoop.h"
#include "Console/ConsoleCommands.h"
#include "Core/Utils.h"
#include <stdlib.h>

#include "AuthGame/TS_GA_LOGOUT.h"
#include "AuthGame/TS_AG_LOGIN_RESULT.h"
#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "PacketEnums.h"

namespace AuthServer {

std::unordered_map<uint16_t, AuthSession*> AuthSession::servers;

void AuthSession::init() {
	ConsoleCommands::get()->addCommand("gameserver.list", "list", 0, 1, &commandList,
									   "List all game servers",
									   "list : list all game servers");
}

AuthSession::AuthSession(GameServerSession *gameServerSession,
						 uint16_t serverIdx,
						 std::string serverName,
						 std::string serverIp,
						 int32_t serverPort,
						 std::string serverScreenshotUrl,
						 bool isAdultServer)
	: gameServerSession(gameServerSession),
	  serverIdx(serverIdx),
	  serverName(serverName),
	  serverIp(serverIp),
	  serverPort(serverPort),
	  serverScreenshotUrl(serverScreenshotUrl),
	  isAdultServer(isAdultServer),
	  creationTime(time(nullptr)),
	  sentLoginPacket(false),
	  pendingLogin(false),
	  synchronizedWithAuth(false)
{
	recoTimer = new uv_timer_t;
	uv_timer_init(EventLoop::getLoop(), recoTimer);
	recoTimer->data = this;
	servers.insert(std::pair<uint16_t, AuthSession*>(serverIdx, this));
}

static void close_timer(uv_handle_t* timer) {
	delete (uv_timer_t*)timer;
}

AuthSession::~AuthSession() {
	uv_timer_stop(recoTimer);
	uv_close((uv_handle_t*)recoTimer, &close_timer);
	servers.erase(serverIdx);
}

void AuthSession::connect() {
	std::string ip = CONFIG_GET()->auth.ip.get();
	uint16_t port = CONFIG_GET()->auth.port.get();
	debug("Connecting to auth %s:%d\n", ip.c_str(), port);

	SocketSession::connect(ip.c_str(), port);
}

void AuthSession::onConnected() {
	getStream()->setKeepAlive(31);

	info("Connected to auth server\n");
	sendLogin();
}

void AuthSession::sendLogin() {
	if(pendingLogin) {
		warn("Already pending GS login to auth, ignore login request\n");
		return;
	}

	pendingLogin = true;

	TS_GA_LOGIN_WITH_LOGOUT loginPacket;
	TS_MESSAGE::initMessage<TS_GA_LOGIN_WITH_LOGOUT>(&loginPacket);

	loginPacket.server_idx = serverIdx;
	strcpy(loginPacket.server_name, serverName.c_str());
	strcpy(loginPacket.server_screenshot_url, serverScreenshotUrl.c_str());
	loginPacket.is_adult_server = isAdultServer;
	strcpy(loginPacket.server_ip, serverIp.c_str());
	loginPacket.server_port = serverPort;

	info("Sending informations for GS %s[%d]\n", loginPacket.server_name, loginPacket.server_idx);
	sendPacketToNetwork(&loginPacket);
}

void AuthSession::onLoginResult(const TS_AG_LOGIN_RESULT* packet) {
	if(!pendingLogin) {
		warn("Received GS login result from auth without pending login, ignore message\n");
		return;
	}
	pendingLogin = false;

	if(gameServerSession)
		gameServerSession->sendPacket(packet);

	if(packet->result == TS_RESULT_SUCCESS) {
		sendAccountList();
	} else {
		warn("Login with auth failed: %d\n", packet->result);
		sentLoginPacket = false;
		if(gameServerSession == nullptr)
			disconnect();
		else if(synchronizedWithAuth == false)
			gameServerSession->disconnectAuth();
	}
}

void AuthSession::sendAccountList() {
	static const int MAX_PACKET_SIZE = 16000;
	static const int MAX_COUNT_PER_PACKET = (MAX_PACKET_SIZE - sizeof(TS_GA_ACCOUNT_LIST)) / sizeof(TS_GA_ACCOUNT_LIST::AccountInfo);

	synchronizedWithAuth = true;

	TS_GA_ACCOUNT_LIST* accountListPacket;
	const int maxCount = (int)(accountList.size() <= MAX_COUNT_PER_PACKET ? accountList.size() : MAX_COUNT_PER_PACKET);
	accountListPacket = TS_MESSAGE_WNA::create<TS_GA_ACCOUNT_LIST, TS_GA_ACCOUNT_LIST::AccountInfo>(maxCount);

	debug("Sending %d accounts\n", (int)accountList.size());

	auto it = accountList.begin();
	do {
		int count = 0;

		for(; it != accountList.end() && count < maxCount; ++it, ++count) {
			const TS_GA_ACCOUNT_LIST::AccountInfo& accountItem = *it;
			TS_GA_ACCOUNT_LIST::AccountInfo* accountInfo = &accountListPacket->accountInfo[count];

			*accountInfo = accountItem;
		}

		if(it == accountList.end())
			accountListPacket->final_packet = true;
		else
			accountListPacket->final_packet = false;
		accountListPacket->count = count;

		sendPacket(accountListPacket);
	} while(it != accountList.end());

	TS_MESSAGE_WNA::destroy(accountListPacket);

	sendPendingMessages();
}

void AuthSession::sendPendingMessages() {
	for(size_t i = 0; i < pendingMessages.size(); i++) {
		TS_MESSAGE* message = pendingMessages.at(i);
		debug("Sending defered message id %d\n", message->id);
		sendPacketToNetwork(message);
		free(message);
	}
	pendingMessages.clear();
}

void AuthSession::onClientLoginResult(const TS_AG_CLIENT_LOGIN_EXTENDED* packet) {
	if(!gameServerSession)
		return;

	debug("Login client %s\n", packet->account);

	if(packet->result == TS_RESULT_SUCCESS) {
		TS_GA_ACCOUNT_LIST::AccountInfo accountInfo;

		memcpy(accountInfo.account, packet->account, sizeof(accountInfo.account));
		accountInfo.ip = packet->ip;
		accountInfo.loginTime = packet->loginTime;
		accountInfo.nAccountID = packet->nAccountID;
		accountInfo.nAge = packet->nAge;
		accountInfo.nEventCode = packet->nEventCode;
		accountInfo.nPCBangUser = packet->nPCBangUser;

		accountList.push_back(accountInfo);
	}

	TS_AG_CLIENT_LOGIN clientLoginOut;
	TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN>(&clientLoginOut);

	memcpy(clientLoginOut.account, packet->account, sizeof(clientLoginOut.account));
	clientLoginOut.nAccountID = packet->nAccountID;
	clientLoginOut.nAge = packet->nAge;
	clientLoginOut.nEventCode = packet->nEventCode;
	clientLoginOut.nPCBangUser = packet->nPCBangUser;
	clientLoginOut.result = packet->result;
	clientLoginOut.nContinuousPlayTime = packet->nContinuousPlayTime;
	clientLoginOut.nContinuousLogoutTime = packet->nContinuousLogoutTime;

	gameServerSession->sendPacket(&clientLoginOut);
}

void AuthSession::logoutClient(const char* account) {
	auto it = accountList.rbegin();
	for(; it != accountList.rend(); ++it) {
		if(strcmp(it->account, account) == 0) {
			debug("Logout client %s\n", account);
			accountList.erase(--(it.base()));
			break;
		}
	}
}

void AuthSession::disconnect() {
	gameServerSession = nullptr;
	pendingMessages.clear(); //pending messages don't matter anymore (GS has disconnected)

	if(sentLoginPacket == false) {
		info("Fast auth disconnect: login message wasn't send\n");
		uv_timer_stop(recoTimer);
		if(isConnected())
			closeSession();
		else
			deleteLater();
	} else {
		TS_GA_LOGOUT logoutPacket;
		TS_MESSAGE::initMessage<TS_GA_LOGOUT>(&logoutPacket);
		sendPacket(&logoutPacket);
	}
}

void AuthSession::forceClose() {
	gameServerSession = nullptr;
	pendingMessages.clear();
	uv_timer_stop(recoTimer);

	if(isConnected()) {
		TS_GA_LOGOUT logoutPacket;
		TS_MESSAGE::initMessage<TS_GA_LOGOUT>(&logoutPacket);
		sendPacket(&logoutPacket);
	} else {
		deleteLater();
	}
}

void AuthSession::onDisconnected(bool causedByRemote) {
	synchronizedWithAuth = false;
	pendingLogin = false;
	if(causedByRemote && (gameServerSession != nullptr || pendingMessages.size() > 0)) {
		int delay = CONFIG_GET()->auth.reconnectDelay.get();
		warn("Disconnected from auth server, reconnecting in %d seconds\n", delay/1000);
		uv_timer_start(recoTimer, &onTimerReconnect, delay, 0);
	} else {
		info("Disconnected from auth server\n");
		if(gameServerSession == nullptr)
			this->deleteLater();
	}
}

void AuthSession::onTimerReconnect(uv_timer_t* timer) {
	AuthSession* session = (AuthSession*) timer->data;
	session->connect();
}

void AuthSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_AG_LOGIN_RESULT::packetID:
			onLoginResult(static_cast<const TS_AG_LOGIN_RESULT*>(packet));
			break;

		case TS_AG_CLIENT_LOGIN_EXTENDED::packetID:
			onClientLoginResult(static_cast<const TS_AG_CLIENT_LOGIN_EXTENDED*>(packet));
			break;

		default:
			if(gameServerSession) {
				debug("Received packet id %d from auth, forwarding to GS\n", packet->id);
				gameServerSession->sendPacket(packet);
			}
	}
}

void AuthSession::sendPacket(const TS_MESSAGE* message) {
	if(isConnected() && synchronizedWithAuth) {
		sendPacketToNetwork(message);
	} else {
		TS_MESSAGE* messageCopy = (TS_MESSAGE*) malloc(message->size);
		memcpy(messageCopy, message, message->size);
		pendingMessages.push_back(messageCopy);
	}
}

void AuthSession::sendPacketToNetwork(const TS_MESSAGE* message) {
	PacketSession::sendPacket(message);
	if(message->id == TS_GA_LOGIN_WITH_LOGOUT::packetID)
		sentLoginPacket = true;
	else if(message->id == TS_GA_LOGOUT::packetID)
		closeSession();
}

void AuthSession::commandList(IWritableConsole* console, const std::vector<std::string>& args) {
	auto& serverList = getServerList();

	if(args.size() < 1) {
		auto it = serverList.begin();
		auto itEnd = serverList.end();
		for(; it != itEnd; ++it) {
			AuthSession* server = it->second;

			struct tm upTime;
			Utils::getGmTime(time(nullptr) - server->getCreationTime(), &upTime);

			console->writef("index: %2d, name: %s, address: %s:%d, players count: %u, uptime: %d:%02d:%02d:%02d, screenshot url: %s\r\n",
							server->getServerIdx(),
							server->getServerName().c_str(),
							server->getServerIp().c_str(),
							server->getServerPort(),
							(int)server->getAccountList().size(),
							(upTime.tm_year - 1970) * 365 + upTime.tm_yday,
							upTime.tm_hour,
							upTime.tm_min,
							upTime.tm_sec,
							server->getServerScreenshotUrl().c_str());
		}
	} else {
		int serverIdx = atoi(args[0].c_str());
		auto authSessionIt = serverList.find(serverIdx);

		if(authSessionIt == serverList.end()) {
			console->writef("No such server index: %d\r\n", serverIdx);
			return;
		}

		AuthSession* authSession = authSessionIt->second;
		auto& accountList = authSession->getAccountList();

		console->writef("Gameserver %s[%d]: %d account(s)\r\n",
						authSession->getServerName().c_str(), authSession->getServerIdx(), (int)accountList.size());

		auto it = accountList.begin();
		auto itEnd = accountList.end();
		for(; it != itEnd; ++it) {
			const TS_GA_ACCOUNT_LIST::AccountInfo& accountInfo = *it;
			char ipStr[16];
			struct tm loginTimeTm;

			uv_inet_ntop(AF_INET, &accountInfo.ip, ipStr, sizeof(ipStr));
			Utils::getGmTime(accountInfo.loginTime, &loginTimeTm);

			console->writef("Account id: %d, name: %s, ip: %s, login time: %d-%02d-%02d %02d:%02d:%02d\r\n",
							accountInfo.nAccountID,
							accountInfo.account,
							ipStr,
							loginTimeTm.tm_year,
							loginTimeTm.tm_mon,
							loginTimeTm.tm_mday,
							loginTimeTm.tm_hour,
							loginTimeTm.tm_min,
							loginTimeTm.tm_sec);
		}
	}
}

} // namespace AuthServer
