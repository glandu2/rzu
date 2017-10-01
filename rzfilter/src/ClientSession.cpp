#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include "Core/PrintfFormats.h"
#include "GlobalConfig.h"
#include "PacketStructsName.h"
#include <algorithm>
#include <string.h>

#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "Packet/PacketEpics.h"
#include "PacketEnums.h"

#include "FilterManager.h"
#include "FilterProxy.h"

ClientSession::ClientSession(bool authMode)
    : serverSession(this), packetFilter(nullptr), version(CONFIG_GET()->client.epic.get()), authMode(authMode) {
	packetFilter = FilterManager::getInstance()->createFilter(this, &serverSession);
}

ClientSession::~ClientSession() {
	FilterManager::getInstance()->destroyFilter(packetFilter);
}

EventChain<SocketSession> ClientSession::onConnected() {
	log(LL_Info, "Client connected, connecting server session\n");

	serverSession.connect(getServerIp(), getServerPort());
	setDirtyObjectName();
	getStream()->setNoDelay(true);

	return PacketSession::onConnected();
}

EventChain<SocketSession> ClientSession::onDisconnected(bool causedByRemote) {
	log(LL_Info, "Client disconnected, disconnecting server session\n");
	serverSession.closeSession();

	return PacketSession::onDisconnected(causedByRemote);
}

void ClientSession::logPacket(bool outgoing, const TS_MESSAGE* msg) {
	const char* packetName = getPacketName(msg->id);

	log(LL_Debug,
	    "%s packet id: %5d, name %s, size: %d\n",
	    (outgoing) ? "SERV->CLI" : "CLI->SERV",
	    msg->id,
	    packetName,
	    int(msg->size - sizeof(TS_MESSAGE)));

	getStream()->packetLog(Object::LL_Debug,
	                       reinterpret_cast<const unsigned char*>(msg) + sizeof(TS_MESSAGE),
	                       (int) msg->size - sizeof(TS_MESSAGE),
	                       "%s packet id: %5d, name %s, size: %d\n",
	                       (outgoing) ? "SERV->CLI" : "CLI->SERV",
	                       msg->id,
	                       packetName,
	                       int(msg->size - sizeof(TS_MESSAGE)));
}

EventChain<PacketSession> ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	// log(LL_Debug, "Received packet id %d from client, forwarding to server\n", packet->id);
	if(packetFilter->onClientPacket(packet, authMode ? IFilter::ST_Auth : IFilter::ST_Game))
		serverSession.sendPacket(packet);

	return PacketSession::onPacketReceived(packet);
}

void ClientSession::onServerPacketReceived(const TS_MESSAGE* packet) {
	// log(LL_Debug, "Received packet id %d from server, forwarding to client\n", packet->id);
	if(packetFilter->onServerPacket(packet, authMode ? IFilter::ST_Auth : IFilter::ST_Game)) {
		sendPacket(packet);
	}
}

const char* ClientSession::getPacketName(int16_t id) {
	return ::getPacketName(id, authMode);
}

void ClientSession::updateObjectName() {
	size_t streamNameSize;
	const char* streamName = getStream()->getObjectName(&streamNameSize);
	setObjectName(8 + (int) streamNameSize, "Client[%s]", streamName);
}
