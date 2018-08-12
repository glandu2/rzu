#include "ServerSession.h"
#include "ClientSession.h"
#include "Core/EventLoop.h"
#include "GlobalConfig.h"
#include "Packet/PacketStructsName.h"
#include <stdlib.h>

ServerSession::ServerSession(ClientSession* clientSession)
    : clientSession(clientSession), version(CONFIG_GET()->server.epic.get()) {}

ServerSession::~ServerSession() {}

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

	setDirtyObjectName();

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

StreamAddress ServerSession::getAddress() {
	if(getStream())
		return getStream()->getRemoteAddress();
	else
		return StreamAddress{};
}

void ServerSession::banAddress(StreamAddress address) {
	clientSession->banAddress(address);
}

EventChain<PacketSession> ServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	// log(LL_Debug, "Received packet id %d from server, forwarding to client\n", packet->id);
	clientSession->onServerPacketReceived(packet);

	return PacketSession::onPacketReceived(packet);
}

void ServerSession::logPacket(bool outgoing, const TS_MESSAGE* msg) {
	if(!CONFIG_GET()->trafficDump.enableServer.get())
		return;

	const char* packetName =
	    ::getPacketName(msg->id,
	                    clientSession->isAuthMode() ? SessionType::AuthClient : SessionType::GameClient,
	                    outgoing ? SessionPacketOrigin::Client : SessionPacketOrigin::Server);

	log(LL_Debug,
	    "%s packet id: %5d, name %s, size: %d\n",
	    (!outgoing) ? "SERV->CLI" : "CLI->SERV",
	    msg->id,
	    packetName,
	    int(msg->size - sizeof(TS_MESSAGE)));

	getStream()->packetLog(Object::LL_Debug,
	                       reinterpret_cast<const unsigned char*>(msg) + sizeof(TS_MESSAGE),
	                       (int) msg->size - sizeof(TS_MESSAGE),
	                       "%s packet id: %5d, name %s, size: %d\n",
	                       (!outgoing) ? "SERV->CLI" : "CLI->SERV",
	                       msg->id,
	                       packetName,
	                       int(msg->size - sizeof(TS_MESSAGE)));
}

void ServerSession::updateObjectName() {
	size_t streamNameSize;
	if(getStream()) {
		const char* streamName = getStream()->getObjectName(&streamNameSize);
		setObjectName(8 + (int) streamNameSize, "Server[%s]", streamName);
	} else {
		Object::updateObjectName();
	}
}
