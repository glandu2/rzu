#include "AuthSession.h"
#include "../GlobalConfig.h"
#include "Console/ConsoleCommands.h"
#include "Core/EventLoop.h"
#include "Core/Utils.h"
#include "GameServerSession.h"
#include <stdlib.h>

#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "AuthGame/TS_AG_LOGIN_RESULT.h"
#include "AuthGame/TS_GA_LOGOUT.h"
#include "PacketEnums.h"

namespace AuthServer {

std::unordered_map<uint16_t, AuthSession*> AuthSession::servers;

void AuthSession::init() {
	ConsoleCommands::get()->addCommand(
	    "gameserver.list", "list", 0, 1, &commandList, "List all game servers", "list : list all game servers");
}

AuthSession::AuthSession(GameServerSession* gameServerSession,
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
      synchronizedWithAuth(false) {
	for(size_t i = 0; i < sizeof(guid); i++) {
		guid[i] = rand() & 0xFF;
	}
	servers.insert(std::pair<uint16_t, AuthSession*>(serverIdx, this));
}

AuthSession::~AuthSession() {
	recoTimer.stop();
	servers.erase(serverIdx);
}

void AuthSession::connect() {
	std::string ip = CONFIG_GET()->auth.ip.get();
	uint16_t port = CONFIG_GET()->auth.port.get();
	log(LL_Debug, "Connecting to auth %s:%d\n", ip.c_str(), port);

	SocketSession::connect(ip.c_str(), port);
}

EventChain<SocketSession> AuthSession::onConnected() {
	getStream()->setKeepAlive(31);

	log(LL_Info, "Connected to auth server\n");
	sendLogin();

	return PacketSession::onConnected();
}

void AuthSession::sendLogin() {
	if(pendingLogin) {
		log(LL_Warning, "Already pending GS login to auth, ignore login request\n");
		return;
	}

	pendingLogin = true;

	TS_GA_LOGIN_WITH_LOGOUT_EXT loginPacket;
	TS_MESSAGE::initMessage<TS_GA_LOGIN_WITH_LOGOUT_EXT>(&loginPacket);

	loginPacket.server_idx = serverIdx;
	strcpy(loginPacket.server_name, serverName.c_str());
	strcpy(loginPacket.server_screenshot_url, serverScreenshotUrl.c_str());
	loginPacket.is_adult_server = isAdultServer;
	strcpy(loginPacket.server_ip, serverIp.c_str());
	loginPacket.server_port = serverPort;

	static_assert(sizeof(loginPacket.guid) == sizeof(guid), "Session GUID and packet GUID must have the same size");
	memcpy(loginPacket.guid, guid, sizeof(guid));

	log(LL_Info, "Sending informations for GS %s[%d]\n", loginPacket.server_name, loginPacket.server_idx);
	sendPacketToNetwork(&loginPacket);
}

void AuthSession::onLoginResult(const TS_AG_LOGIN_RESULT* packet) {
	if(!pendingLogin) {
		log(LL_Warning, "Received GS login result from auth without pending login, ignore message\n");
		return;
	}
	pendingLogin = false;

	if(gameServerSession)
		gameServerSession->sendPacket(packet);

	if(packet->result == TS_RESULT_SUCCESS) {
		sendAccountList();
	} else {
		log(LL_Warning, "Login with auth failed: %d\n", packet->result);
		sentLoginPacket = false;
		if(gameServerSession == nullptr)
			disconnect();
		else if(synchronizedWithAuth == false)
			gameServerSession->disconnectAuth();
	}
}

void AuthSession::sendAccountList() {
	static const int MAX_PACKET_SIZE = 16000;
	static const int MAX_COUNT_PER_PACKET =
	    (MAX_PACKET_SIZE - sizeof(TS_GA_ACCOUNT_LIST)) / sizeof(TS_GA_ACCOUNT_LIST::AccountInfo);

	synchronizedWithAuth = true;

	TS_GA_ACCOUNT_LIST* accountListPacket;
	const int maxCount = (int) (accountList.size() <= MAX_COUNT_PER_PACKET ? accountList.size() : MAX_COUNT_PER_PACKET);
	accountListPacket = TS_MESSAGE_WNA::create<TS_GA_ACCOUNT_LIST, TS_GA_ACCOUNT_LIST::AccountInfo>(maxCount);

	log(LL_Debug, "Sending %d accounts\n", (int) accountList.size());

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
		log(LL_Debug, "Sending defered message id %d\n", message->id);
		sendPacketToNetwork(message);
		free(message);
	}
	pendingMessages.clear();
}

void AuthSession::onClientLoginResult(const TS_AG_CLIENT_LOGIN_EXTENDED* packet) {
	if(!gameServerSession)
		return;

	log(LL_Debug, "Login client %s\n", packet->account);

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

void AuthSession::logoutClient(const TS_GA_CLIENT_LOGOUT* packet) {
	auto it = accountList.rbegin();
	for(; it != accountList.rend(); ++it) {
		const TS_GA_ACCOUNT_LIST::AccountInfo& accountInfo = *it;
		if(strcmp(accountInfo.account, packet->account) == 0) {
			log(LL_Debug, "Logout client %s\n", packet->account);
			accountList.erase(--(it.base()));
			break;
		}
	}

	if(isConnected() && synchronizedWithAuth) {
		sendPacket(packet);
	}
}

void AuthSession::disconnect() {
	gameServerSession = nullptr;
	pendingMessages.clear();  // pending messages don't matter anymore (GS has disconnected)

	if(sentLoginPacket == false) {
		log(LL_Info, "Fast auth disconnect: login message wasn't send\n");
		recoTimer.stop();
		closeSession();
		if(!isConnected())
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
	recoTimer.stop();

	if(isConnected()) {
		TS_GA_LOGOUT logoutPacket;
		TS_MESSAGE::initMessage<TS_GA_LOGOUT>(&logoutPacket);
		sendPacket(&logoutPacket);
	} else {
		closeSession();
		deleteLater();
	}
}

EventChain<SocketSession> AuthSession::onDisconnected(bool causedByRemote) {
	synchronizedWithAuth = false;
	pendingLogin = false;
	if(causedByRemote && (gameServerSession != nullptr || pendingMessages.size() > 0)) {
		int delay = CONFIG_GET()->auth.reconnectDelay.get();
		log(LL_Warning, "Disconnected from auth server, reconnecting in %d seconds\n", delay / 1000);
		recoTimer.start(this, &AuthSession::onTimerReconnect, delay, 0);
	} else {
		log(LL_Info, "Disconnected from auth server\n");
		if(gameServerSession == nullptr)
			this->deleteLater();
	}

	return PacketSession::onDisconnected(causedByRemote);
}

void AuthSession::onTimerReconnect() {
	connect();
}

EventChain<PacketSession> AuthSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_AG_LOGIN_RESULT::packetID:
			onLoginResult(static_cast<const TS_AG_LOGIN_RESULT*>(packet));
			break;

		case TS_AG_CLIENT_LOGIN_EXTENDED::packetID:
			onClientLoginResult(static_cast<const TS_AG_CLIENT_LOGIN_EXTENDED*>(packet));
			break;

		default:
			if(gameServerSession) {
				log(LL_Debug, "Received packet id %d from auth, forwarding to GS\n", packet->id);
				gameServerSession->sendPacket(packet);
			}
	}

	return PacketSession::onPacketReceived(packet);
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
	if(message->id == TS_GA_LOGIN_WITH_LOGOUT_EXT::packetID)
		sentLoginPacket = true;
	else if(message->id == TS_GA_LOGOUT::packetID)
		closeSession();
}

void AuthSession::commandList(IWritableConsole* console, const std::vector<std::string>& args) {
	if(args.size() < 1) {
		auto it = servers.begin();
		auto itEnd = servers.end();
		for(; it != itEnd; ++it) {
			const std::pair<uint16_t, AuthSession*>& serverElement = *it;
			AuthSession* server = serverElement.second;

			struct tm upTime;
			Utils::getGmTime(time(nullptr) - server->getCreationTime(), &upTime);

			console->writef("index: %2d, name: %s, address: %s:%d, players count: %u, uptime: %d:%02d:%02d:%02d, "
			                "screenshot url: %s\r\n",
			                server->getServerIdx(),
			                server->getServerName().c_str(),
			                server->getServerIp().c_str(),
			                server->getServerPort(),
			                (int) server->getAccountList().size(),
			                (upTime.tm_year - 1970) * 365 + upTime.tm_yday,
			                upTime.tm_hour,
			                upTime.tm_min,
			                upTime.tm_sec,
			                server->getServerScreenshotUrl().c_str());
		}
	} else {
		int serverIdx = atoi(args[0].c_str());
		auto authSessionIt = servers.find(serverIdx);

		if(authSessionIt == servers.end()) {
			console->writef("No such server index: %d\r\n", serverIdx);
			return;
		}

		const std::pair<uint16_t, AuthSession*>& serverElement = *authSessionIt;
		AuthSession* authSession = serverElement.second;
		auto& accountList = authSession->getAccountList();

		console->writef("Gameserver %s[%d]: %d account(s)\r\n",
		                authSession->getServerName().c_str(),
		                authSession->getServerIdx(),
		                (int) accountList.size());

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

}  // namespace AuthServer
