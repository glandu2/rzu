#include "AuthSession.h"
#include "GameServerSession.h"
#include "../GlobalConfig.h"
#include "EventLoop.h"

#include "Packets/TS_GA_LOGOUT.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/PacketEnums.h"

namespace AuthServer {

std::unordered_map<uint16_t, AuthSession*> AuthSession::servers;

AuthSession::AuthSession(GameServerSession *gameServerSession)
	: gameServerSession(gameServerSession),
	  serverIdx(UINT16_MAX),
	  serverPort(0),
	  isAdultServer(false),
	  disconnectRequested(false),
	  sentLoginPacket(false),
	  pendingLogin(false),
	  serverInfoValid(false),
	  synchronizedWithAuth(false),
	  uniqueIdentifier(0)
{
	recoTimer = new uv_timer_t;
	uv_timer_init(EventLoop::getLoop(), recoTimer);
	recoTimer->data = this;
	while(uniqueIdentifier == 0)
		uniqueIdentifier = ((uint64_t)rand())*rand()*rand()*rand();
}

static void close_timer(uv_handle_t* timer) {
	delete (uv_timer_t*)timer;
}

AuthSession::~AuthSession() {
	uv_timer_stop(recoTimer);
	uv_close((uv_handle_t*)recoTimer, &close_timer);
	if(serverInfoValid)
		servers.erase(serverIdx);
}

void AuthSession::connect() {
	std::string ip = CONFIG_GET()->auth.ip.get();
	uint16_t port = CONFIG_GET()->auth.port.get();
	debug("Connecting to auth %s:%d\n", ip.c_str(), port);

	SocketSession::connect(ip.c_str(), port);
}

void AuthSession::disconnect() {
	TS_GA_LOGOUT logoutPacket;

	disconnectRequested = true;
	gameServerSession = nullptr;

	if(sentLoginPacket == false && !isConnected()) {
		debug("Fast auth disconnect: login message wasn't send");
		pendingMessages.clear();
		uv_timer_stop(recoTimer);
		this->deleteLater();
	} else {
		TS_MESSAGE::initMessage<TS_GA_LOGOUT>(&logoutPacket);
		sendPacket(&logoutPacket);

		if(isConnected())
			closeSession();
	}
}

void AuthSession::forceClose() {
	TS_GA_LOGOUT logoutPacket;

	TS_MESSAGE::initMessage<TS_GA_LOGOUT>(&logoutPacket);
	sendPacket(&logoutPacket);

	pendingMessages.clear();
	disconnectRequested = true;
	gameServerSession = nullptr;
	uv_timer_stop(recoTimer);

	if(isConnected())
		closeSession();
	else
		this->deleteLater();
}

void AuthSession::onConnected() {
	getStream()->setKeepAlive(31);

	info("Connected to auth server\n");
	if(!disconnectRequested)
		sendLogin();
}

void AuthSession::onDisconnected(bool causedByRemote) {
	synchronizedWithAuth = false;
	if(disconnectRequested == false || pendingMessages.size() > 0) {
		warn("Disconnected from auth server, reconnecting in 5 seconds\n");
		uv_timer_start(recoTimer, &onTimerReconnect, 5000, 0);
	} else {
		info("Disconnected from auth server\n");
		this->deleteLater();
	}
}

void AuthSession::onPacketReceived(const TS_MESSAGE* packet) {

	switch(packet->id) {
		case TS_AG_LOGIN_RESULT::packetID:
			if(pendingLogin && gameServerSession)
				gameServerSession->sendPacket(packet);

			pendingLogin = false;

			if(static_cast<const TS_AG_LOGIN_RESULT*>(packet)->result == TS_RESULT_SUCCESS) {
				serverInfoValid = true;
				servers.insert(std::pair<uint16_t, AuthSession*>(serverIdx, this));
				sendAccountList();
			} else {
				warn("Login with auth failed: %d\n", static_cast<const TS_AG_LOGIN_RESULT*>(packet)->result);
				sentLoginPacket = false;
			}

			if(disconnectRequested)
				closeSession();
			break;

		case TS_AG_CLIENT_LOGIN_EXTENDED::packetID: {
			TS_GA_CLIENT_LOGGED_LIST::AccountInfo accountInfo;
			const TS_AG_CLIENT_LOGIN_EXTENDED* clientLoginIn = static_cast<const TS_AG_CLIENT_LOGIN_EXTENDED*>(packet);

			debug("Login client %s\n", clientLoginIn->account);


			strcpy(accountInfo.account, clientLoginIn->account);
			accountInfo.ip = clientLoginIn->ip;
			accountInfo.loginTime = clientLoginIn->loginTime;
			accountInfo.nAccountID = clientLoginIn->nAccountID;
			accountInfo.nAge = clientLoginIn->nAge;
			accountInfo.nEventCode = clientLoginIn->nEventCode;
			accountInfo.nPCBangUser = clientLoginIn->nPCBangUser;

			accountList.push_back(accountInfo);

			if(!gameServerSession)
				break;

			TS_AG_CLIENT_LOGIN clientLoginOut;
			TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN>(&clientLoginOut);

			strcpy(clientLoginOut.account, clientLoginIn->account);
			clientLoginOut.nAccountID = clientLoginIn->nAccountID;
			clientLoginOut.nAge = clientLoginIn->nAge;
			clientLoginOut.nEventCode = clientLoginIn->nEventCode;
			clientLoginOut.nPCBangUser = clientLoginIn->nPCBangUser;
			clientLoginOut.result = clientLoginIn->result;
			clientLoginOut.nContinuousPlayTime = clientLoginIn->nContinuousPlayTime;
			clientLoginOut.nContinuousLogoutTime = clientLoginIn->nContinuousLogoutTime;

			gameServerSession->sendPacket(&clientLoginOut);
			break;
		}

		case TS_CC_EVENT::packetID:
			break;

		default:
			if(gameServerSession) {
				debug("Received packet id %d from auth, forwarding to GS\n", packet->id);
				gameServerSession->sendPacket(packet);
			}
	}
}

bool AuthSession::loginServer(uint16_t serverIdx, std::string serverName, std::string serverIp, int32_t serverPort, std::string serverScreenshotUrl, bool isAdultServer) {
	if(pendingLogin || serverInfoValid)
		return false;

	this->serverIdx = serverIdx;
	this->serverName = serverName;
	this->serverIp = serverIp;
	this->serverPort = serverPort;
	this->serverScreenshotUrl = serverScreenshotUrl;
	this->isAdultServer = isAdultServer;

	setDirtyObjectName();
	pendingLogin = true;

	debug("GS login requested\n");

	if(isConnected())
		sendLogin();

	return true;
}

void AuthSession::sendLogin() {
	if(!serverInfoValid && !pendingLogin)
		return;

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

void AuthSession::sendAccountList() {
	const int MAXCOUNT_PER_PACKET = 2;

	synchronizedWithAuth = true;

	TS_GA_CLIENT_LOGGED_LIST* accountListPacket;
	size_t count = accountList.size() <= MAXCOUNT_PER_PACKET ? accountList.size() : MAXCOUNT_PER_PACKET;
	accountListPacket = TS_MESSAGE_WNA::create<TS_GA_CLIENT_LOGGED_LIST, TS_GA_CLIENT_LOGGED_LIST::AccountInfo>(count);
	accountListPacket->count = 0;
	accountListPacket->final_packet = false;

	debug("Sending %d accounts\n", (int)accountList.size());

	if(accountList.size() == 0) {
		accountListPacket->final_packet = true;
		sendPacket(accountListPacket);
	} else {
		size_t i = 0;
		auto it = accountList.begin();
		for(; it != accountList.end(); ++it) {
			TS_GA_CLIENT_LOGGED_LIST::AccountInfo* accountInfo = &(accountListPacket->accountInfo[accountListPacket->count]);
			const TS_GA_CLIENT_LOGGED_LIST::AccountInfo& accountItem = *it;

			*accountInfo = accountItem;

			accountListPacket->count++;
			i++;

			if(it == --accountList.end())
				accountListPacket->final_packet = true;

			if(accountListPacket->count >= count)
				sendPacket(accountListPacket);

			if(it == --accountList.end() && accountListPacket->count >= count) {
				TS_MESSAGE_WNA::destroy(accountListPacket);

				count = (accountList.size() - i) <= MAXCOUNT_PER_PACKET ? (accountList.size() - i) : MAXCOUNT_PER_PACKET;
				accountListPacket = TS_MESSAGE_WNA::create<TS_GA_CLIENT_LOGGED_LIST, TS_GA_CLIENT_LOGGED_LIST::AccountInfo>(count);
				accountListPacket->count = 0;
				accountListPacket->final_packet = false;
			}
		}
	}

	TS_MESSAGE_WNA::destroy(accountListPacket);

	for(size_t i = 0; i < pendingMessages.size(); i++) {
		TS_MESSAGE* message = pendingMessages.at(i);
		debug("Sending defered message id %d\n", message->id);
		sendPacketToNetwork(message);
		free(message);
	}
	pendingMessages.clear();
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

void AuthSession::onTimerReconnect(uv_timer_t* timer) {
	AuthSession* session = (AuthSession*) timer->data;

	session->connect();
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
	if(message->id == TS_GA_LOGIN_WITH_LOGOUT::packetID)
		sentLoginPacket = true;
	PacketSession::sendPacket(message);
}

void AuthSession::updateObjectName() {
	setObjectName(13 + getServerName().size(), "AuthSession[%s]", getServerName().c_str());
}

} // namespace AuthServer
