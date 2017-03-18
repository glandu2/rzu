#include "ServerSession.h"
#include "ClientSession.h"
#include "GlobalConfig.h"
#include "Core/EventLoop.h"
#include <stdlib.h>

ServerSession::ServerSession(ClientSession *clientSession)
	: clientSession(clientSession), version(CONFIG_GET()->server.epic.get())
{
}

ServerSession::~ServerSession() {
}

void ServerSession::connect(std::string ip, uint16_t port) {
	log(LL_Debug, "Connecting to server %s:%d\n", ip.c_str(), port);
	SocketSession::connect(ip.c_str(), port);
	if(getStream() && clientSession->getStream()) {
		getStream()->setPacketLogger(clientSession->getStream()->getPacketLogger());
	}
}

EventChain<SocketSession> ServerSession::onConnected() {
	log(LL_Info, "Connected to server\n");
	getStream()->setNoDelay(true);

	for(size_t i = 0; i < pendingMessages.size(); i++) {
		TS_MESSAGE* message = pendingMessages.at(i);
		log(LL_Info, "Sending defered message id %d\n", message->id);
		PacketSession::sendPacket(message);
		free(message);
	}
	pendingMessages.clear();

	return PacketSession::onConnected();
}

EventChain<SocketSession> ServerSession::onDisconnected(bool causedByRemote) {
	log(LL_Warning, "Disconnected from server\n");
	clientSession->closeSession();

	return PacketSession::onDisconnected(causedByRemote);
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

EventChain<PacketSession> ServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	//log(LL_Debug, "Received packet id %d from server, forwarding to client\n", packet->id);
	clientSession->onServerPacketReceived(packet);

	return PacketSession::onPacketReceived(packet);
}
