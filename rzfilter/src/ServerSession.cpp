#include "ServerSession.h"
#include "ClientSession.h"
#include "Core/EventLoop.h"
#include "GlobalConfig.h"
#include "Packet/PacketStructsName.h"
#include <stdlib.h>

#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "GameClientSessionManager.h"
#include <algorithm>

#include "FilterManager.h"
#include "FilterProxy.h"

ServerSession::ServerSession(bool authMode,
                             ClientSession* clientSession,
                             GameClientSessionManager* gameClientSessionManager,
                             FilterManager* filterManager,
                             FilterManager* converterFilterManager)
    : EncryptedSession<PacketSession>(authMode ? SessionType::AuthClient : SessionType::GameClient,
                                      SessionPacketOrigin::Client,
                                      CONFIG_GET()->server.epic.get()),
      authMode(authMode),
      clientSession(clientSession),
      gameClientSessionManager(gameClientSessionManager),
      clientEndpointProxy(clientSession) {
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
		packetFilter->bindEndpoints(packetConverterFilter->getToClientEndpoint(), this);
		packetConverterFilter->bindEndpoints(&clientEndpointProxy, packetFilter->getToServerEndpoint());
		toServerBaseEndpoint = packetConverterFilter->getToServerEndpoint();
		toClientBaseEndpoint = packetFilter->getToClientEndpoint();
	} else if(packetFilter) {
		packetFilter->bindEndpoints(&clientEndpointProxy, this);
		toServerBaseEndpoint = packetFilter->getToServerEndpoint();
		toClientBaseEndpoint = packetFilter->getToClientEndpoint();
	} else if(packetConverterFilter) {
		packetConverterFilter->bindEndpoints(&clientEndpointProxy, this);
		toServerBaseEndpoint = packetConverterFilter->getToServerEndpoint();
		toClientBaseEndpoint = packetConverterFilter->getToClientEndpoint();
	} else {
		toServerBaseEndpoint = this;
		toClientBaseEndpoint = &clientEndpointProxy;
	}
}

ServerSession::~ServerSession() {}

void ServerSession::connect(std::string ip, uint16_t port) {
	log(LL_Debug, "Connecting to server %s:%d\n", ip.c_str(), port);
	SocketSession::connect(ip.c_str(), port);
	if(getStream() && clientSession && clientSession->getStream()) {
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
	if(clientSession) {
		clientSession->detachServer();
		clientSession->closeSession();
	}

	deleteLater();

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
	if(clientSession)
		clientSession->banAddress(address);
	else {
		char buffer[128];
		address.getName(buffer, sizeof(buffer));
		log(LL_Warning, "Can't ban IP %s, client has disconnected\n", buffer);
	}
}

bool ServerSession::isStrictForwardEnabled() {
	return CONFIG_GET()->server.strictforward.get();
}

void ServerSession::onClientPacketReceived(const TS_MESSAGE* packet) {
	toServerBaseEndpoint->sendPacket(packet);
}

void ServerSession::onClientDisconnected() {
	toServerBaseEndpoint->close();
}

void ServerSession::detachClient() {
	clientSession = nullptr;
	clientEndpointProxy.detach();
}

EventChain<PacketSession> ServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	if(authMode && packet->id == TS_AC_SERVER_LIST::packetID && clientSession && gameClientSessionManager) {
		TS_AC_SERVER_LIST serverListPkt;
		if(packet->process(serverListPkt, getPacketVersion())) {
			std::string listenIp = CONFIG_GET()->client.listener.listenIp.get();
			std::vector<TS_SERVER_INFO*> serversOrdered;

			serversOrdered.reserve(serverListPkt.servers.size());
			for(size_t i = 0; i < serverListPkt.servers.size(); i++) {
				serversOrdered.push_back(&serverListPkt.servers[i]);
			}

			std::sort(serversOrdered.begin(), serversOrdered.end(), [](TS_SERVER_INFO* a, TS_SERVER_INFO* b) -> bool {
				return a->server_idx < b->server_idx;
			});

			uint16_t baseListenPort = CONFIG_GET()->client.gameBasePort.get();
			for(size_t i = 0; i < serversOrdered.size(); i++) {
				TS_SERVER_INFO& server = *serversOrdered[i];
				uint16_t listenPort = gameClientSessionManager->ensureListening(
				    listenIp,
				    baseListenPort,
				    server.server_ip,
				    server.server_port,
				    clientSession->getStream() ? clientSession->getStream()->getPacketLogger() : nullptr,
				    clientSession->getBanManager());
				if(!listenPort) {
					log(LL_Error,
					    "Failed to listen on %s for server %s:%d\n",
					    listenIp.c_str(),
					    server.server_ip.c_str(),
					    server.server_port);
				} else {
					log(LL_Debug,
					    "Listening on %s:%d for server %s:%d\n",
					    listenIp.c_str(),
					    listenPort,
					    server.server_ip.c_str(),
					    server.server_port);
				}
				server.server_ip = CONFIG_GET()->client.gameExternalIp.get();
				server.server_port = listenPort;

				// If not using random ports (ie: port 0), increment the base port for subsequent GS
				if(baseListenPort)
					baseListenPort++;
			}
		}
		toClientBaseEndpoint->sendPacket(serverListPkt);
	} else {
		// log(LL_Debug, "Received packet id %d from server, forwarding to client\n", packet->id);
		toClientBaseEndpoint->sendPacket(packet);
	}

	return PacketSession::onPacketReceived(packet);
}

void ServerSession::logPacket(bool outgoing, const TS_MESSAGE* msg) {
	if(!CONFIG_GET()->trafficDump.enableServer.get())
		return;

	EncryptedSession<PacketSession>::logPacket(outgoing, msg);
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
