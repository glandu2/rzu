#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include "Core/PrintfFormats.h"
#include "GlobalConfig.h"
#include "Packet/PacketStructsName.h"
#include <algorithm>
#include <string.h>

#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "Packet/PacketEpics.h"
#include "PacketEnums.h"

#include "FilterManager.h"
#include "FilterProxy.h"

ClientSession::ClientSession(bool authMode, FilterManager* filterManager, FilterManager* converterFilterManager)
    : serverSession(this), packetFilter(nullptr), version(CONFIG_GET()->client.epic.get()), authMode(authMode) {
	packetFilter = filterManager->createFilter(authMode ? IFilter::ST_Auth : IFilter::ST_Game);
	packetConverterFilter = converterFilterManager->createFilter(authMode ? IFilter::ST_Auth : IFilter::ST_Game);

	packetFilter->bindEndpoints(packetConverterFilter->getToClientEndpoint(), &serverSession);
	packetConverterFilter->bindEndpoints(this, packetFilter->getToServerEndpoint());
}

ClientSession::~ClientSession() {}

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

void ClientSession::logPacket(bool toClient, const TS_MESSAGE* msg) {
	const char* packetName = ::getPacketName(msg->id,
	                                         authMode ? SessionType::AuthClient : SessionType::GameClient,
	                                         toClient ? SessionPacketOrigin::Server : SessionPacketOrigin::Client);

	log(LL_Debug,
	    "%s packet id: %5d, name %s, size: %d\n",
	    (toClient) ? "SERV->CLI" : "CLI->SERV",
	    msg->id,
	    packetName,
	    int(msg->size - sizeof(TS_MESSAGE)));

	getStream()->packetLog(Object::LL_Debug,
	                       reinterpret_cast<const unsigned char*>(msg) + sizeof(TS_MESSAGE),
	                       (int) msg->size - sizeof(TS_MESSAGE),
	                       "%s packet id: %5d, name %s, size: %d\n",
	                       (toClient) ? "SERV->CLI" : "CLI->SERV",
	                       msg->id,
	                       packetName,
	                       int(msg->size - sizeof(TS_MESSAGE)));
}

EventChain<PacketSession> ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	// log(LL_Debug, "Received packet id %d from client, forwarding to server\n", packet->id);
	packetConverterFilter->onClientPacket(packet);

	return PacketSession::onPacketReceived(packet);
}

void ClientSession::onServerPacketReceived(const TS_MESSAGE* packet) {
	// log(LL_Debug, "Received packet id %d from server, forwarding to client\n", packet->id);
	packetFilter->onServerPacket(packet);
}

void ClientSession::updateObjectName() {
	size_t streamNameSize;
	const char* streamName = getStream()->getObjectName(&streamNameSize);
	setObjectName(8 + (int) streamNameSize, "Client[%s]", streamName);
}
