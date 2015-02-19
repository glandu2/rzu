#include "AuthSession.h"
#include "GameServerSession.h"
#include "../GlobalConfig.h"
#include "EventLoop.h"

#include "Packets/TS_GA_LOGOUT.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"

namespace AuthServer {

std::unordered_map<uint16_t, AuthSession*> AuthSession::servers;

AuthSession::AuthSession(GameServerSession *gameServerSession, uint16_t serverIdx, std::string serverName, std::string serverIp, int32_t serverPort, std::string serverScreenshotUrl, bool isAdultServer)
	: gameServerSession(gameServerSession),
	  serverIdx(serverIdx),
	  serverName(serverName),
	  serverIp(serverIp),
	  serverPort(serverPort),
	  serverScreenshotUrl(serverScreenshotUrl),
	  isAdultServer(isAdultServer),
	  disconnectRequested(false),
	  sentLoginPacket(false),
	  loginResultReceived(false)
{
	uv_timer_init(EventLoop::getLoop(), &recoTimer);
	recoTimer.data = this;
	servers.insert(std::pair<uint16_t, AuthSession*>(serverIdx, this));
}

AuthSession::~AuthSession() {
	uv_timer_stop(&recoTimer);
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

	if(sentLoginPacket == false && !isConnected()) {
		pendingMessages.clear();
		disconnectRequested = true;
		uv_timer_stop(&recoTimer);
		this->deleteLater();
	}

	TS_MESSAGE::initMessage<TS_GA_LOGOUT>(&logoutPacket);
	sendPacket(&logoutPacket);

	disconnectRequested = true;
	gameServerSession = nullptr;

	if(isConnected())
		closeSession();
}

void AuthSession::forceClose() {
	TS_GA_LOGOUT logoutPacket;

	TS_MESSAGE::initMessage<TS_GA_LOGOUT>(&logoutPacket);
	sendPacket(&logoutPacket);

	pendingMessages.clear();
	disconnectRequested = true;
	gameServerSession = nullptr;
	uv_timer_stop(&recoTimer);

	if(isConnected())
		closeSession();
	else
		this->deleteLater();
}

void AuthSession::onConnected() {
	TS_GA_LOGIN_WITH_LOGOUT loginPacket;
	TS_MESSAGE::initMessage<TS_GA_LOGIN_WITH_LOGOUT>(&loginPacket);

	getStream()->setKeepAlive(31);

	loginPacket.server_idx = serverIdx;
	strcpy(loginPacket.server_name, serverName.c_str());
	strcpy(loginPacket.server_screenshot_url, serverScreenshotUrl.c_str());
	loginPacket.is_adult_server = isAdultServer;
	strcpy(loginPacket.server_ip, serverIp.c_str());
	loginPacket.server_port = serverPort;

	info("Connected to auth server, sending informations for GS %s, id %d\n", loginPacket.server_name, loginPacket.server_idx);
	sendPacket(&loginPacket);

	sentLoginPacket = true;

	for(size_t i = 0; i < pendingMessages.size(); i++) {
		TS_MESSAGE* message = pendingMessages.at(i);
		info("Sending defered message id %d\n", message->id);
		PacketSession::sendPacket(message);
		free(message);
	}
	pendingMessages.clear();

	if(disconnectRequested)
		closeSession();
}

void AuthSession::onDisconnected(bool causedByRemote) {
	if(disconnectRequested == false || pendingMessages.size() > 0) {
		warn("Disconnected from auth server, reconnecting in 5 seconds\n");
		uv_timer_start(&recoTimer, &onTimerReconnect, 5000, 0);
	} else {
		info("Disconnected successfuly from auth server\n");
		this->deleteLater();
	}
}

void AuthSession::onTimerReconnect(uv_timer_t* timer) {
	AuthSession* session = (AuthSession*) timer->data;

	session->connect();
}

void AuthSession::sendPacket(const TS_MESSAGE* message) {
	if(isConnected()) {
		PacketSession::sendPacket(message);
	} else {
		TS_MESSAGE* messageCopy = (TS_MESSAGE*) malloc(message->size);
		memcpy(messageCopy, message, message->size);
		pendingMessages.push_back(messageCopy);
	}
}

void AuthSession::onPacketReceived(const TS_MESSAGE* packet) {
	if(!gameServerSession)
		return;

	switch(packet->id) {
		case TS_AG_LOGIN_RESULT::packetID:
			if(!loginResultReceived) {
				loginResultReceived = true;
				gameServerSession->sendPacket(packet);
			}
			break;

		case TS_CC_EVENT::packetID:
			break;

		default:
			debug("Received packet id %d from auth, forwarding to GS\n", packet->id);
			gameServerSession->sendPacket(packet);
	}
}

} // namespace AuthServer
