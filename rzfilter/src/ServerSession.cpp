#include "ServerSession.h"
#include "ClientSession.h"
#include "GlobalConfig.h"
#include "Core/EventLoop.h"

ServerSession::ServerSession(ClientSession *clientSession)
	: clientSession(clientSession)
{
}

ServerSession::~ServerSession() {
}

void ServerSession::connect() {
	std::string ip = CONFIG_GET()->server.ip.get();
	uint16_t port = CONFIG_GET()->server.port.get();
	debug("Connecting to server %s:%d\n", ip.c_str(), port);
	SocketSession::connect(ip.c_str(), port);
}

void ServerSession::onConnected() {
	info("Connected to server\n");
	getStream()->setNoDelay(true);

	for(size_t i = 0; i < pendingMessages.size(); i++) {
		TS_MESSAGE* message = pendingMessages.at(i);
		info("Sending defered message id %d\n", message->id);
		PacketSession::sendPacket(message);
		free(message);
	}
	pendingMessages.clear();
}

void ServerSession::onDisconnected(bool causedByRemote) {
	warn("Disconnected from server\n");
	clientSession->closeSession();
}

void ServerSession::sendPacket(const TS_MESSAGE* message) {
	if(getStream() && getStream()->getState() == Stream::ConnectedState) {
		PacketSession::sendPacket(message);
	} else {
		TS_MESSAGE* messageCopy = (TS_MESSAGE*) malloc(message->size);
		memcpy(messageCopy, message, message->size);
		pendingMessages.push_back(messageCopy);
	}
}

void ServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	debug("Received packet id %d from server, forwarding to client\n", packet->id);
	clientSession->onServerPacketReceived(packet);
}
