#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include "Core/PrintfFormats.h"
#include "GlobalConfig.h"
#include "NetSession/BanManager.h"
#include "NetSession/SessionServer.h"
#include "Packet/PacketStructsName.h"
#include <algorithm>
#include <string.h>

#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "Packet/PacketEpics.h"
#include "PacketEnums.h"

#include "FilterManager.h"
#include "FilterProxy.h"

ClientSession::ClientSession(bool authMode, FilterManager* filterManager, FilterManager* converterFilterManager)
    : serverSession(this), version(CONFIG_GET()->client.epic.get()), authMode(authMode) {
	if(filterManager)
		packetFilter = filterManager->createFilter(authMode ? IFilter::ST_Auth : IFilter::ST_Game);
	else
		packetFilter = nullptr;

	if(converterFilterManager)
		packetConverterFilter = converterFilterManager->createFilter(authMode ? IFilter::ST_Auth : IFilter::ST_Game);
	else
		packetConverterFilter = nullptr;

	// Client <=> version conversion filter <=> user filter <=> Server
	if(packetFilter && packetConverterFilter) {
		packetFilter->bindEndpoints(packetConverterFilter->getToClientEndpoint(), &serverSession);
		packetConverterFilter->bindEndpoints(this, packetFilter->getToServerEndpoint());
		toServerBaseEndpoint = packetConverterFilter->getToServerEndpoint();
		toClientBaseEndpoint = packetFilter->getToClientEndpoint();
	} else if(packetFilter) {
		packetFilter->bindEndpoints(this, &serverSession);
		toServerBaseEndpoint = packetFilter->getToServerEndpoint();
		toClientBaseEndpoint = packetFilter->getToClientEndpoint();
	} else if(packetConverterFilter) {
		packetConverterFilter->bindEndpoints(this, &serverSession);
		toServerBaseEndpoint = packetConverterFilter->getToServerEndpoint();
		toClientBaseEndpoint = packetConverterFilter->getToClientEndpoint();
	} else {
		toServerBaseEndpoint = &serverSession;
		toClientBaseEndpoint = this;
	}
}

StreamAddress ClientSession::getAddress() {
	if(getStream())
		return getStream()->getRemoteAddress();
	else
		return StreamAddress{};
}

void ClientSession::banAddress(StreamAddress address) {
	if(getServer() && getServer()->getBanManager())
		getServer()->getBanManager()->banIp(address);
}

bool ClientSession::isStrictForwardEnabled() {
	return CONFIG_GET()->client.strictforward.get();
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
	toServerBaseEndpoint->sendPacket(packet);

	return PacketSession::onPacketReceived(packet);
}

void ClientSession::onServerPacketReceived(const TS_MESSAGE* packet) {
	// log(LL_Debug, "Received packet id %d from server, forwarding to client\n", packet->id);
	toClientBaseEndpoint->sendPacket(packet);
}

void ClientSession::updateObjectName() {
	size_t streamNameSize;
	const char* streamName = getStream()->getObjectName(&streamNameSize);
	setObjectName(8 + (int) streamNameSize, "Client[%s]", streamName);
}
