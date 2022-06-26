#include "ServerSession.h"
#include "ClientSession.h"
#include "Core/EventLoop.h"
#include "GlobalConfig.h"
#include "Packet/PacketStructsName.h"
#include <charconv>
#include <iterator>
#include <sstream>
#include <stdlib.h>

#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "GameClient/TS_SC_URL_LIST.h"
#include "GameClientSessionManager.h"
#include <algorithm>

#include "FilterManager.h"
#include "FilterProxy.h"

uint16_t ServerSession::uploadBasePort = 0;

ServerSession::ServerSession(SessionType sessionType,
                             ClientSession* clientSession,
                             GameClientSessionManager* gameClientSessionManager,
                             FilterManager* filterManager,
                             FilterManager* converterFilterManager)
    : EncryptedSession<PacketSession>(sessionType, SessionPacketOrigin::Client, CONFIG_GET()->server.epic.get()),
      sessionType(sessionType),
      clientSession(clientSession),
      gameClientSessionManager(gameClientSessionManager),
      clientEndpointProxy(clientSession) {
	if(filterManager)
		packetFilter = filterManager->createFilter(sessionType);
	else
		packetFilter = nullptr;

	if(converterFilterManager)
		packetConverterFilter = converterFilterManager->createFilter(sessionType);
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

void ServerSession::assignStream(Stream* stream) {
	EncryptedSession<PacketSession>::assignStream(stream);

	if(!localHostBindIp.empty()) {
		getStream()->bindBeforeConnect(localHostBindIp, 0);
	}
}

void ServerSession::connect(std::string ip, uint16_t port, StreamAddress clientIp) {
	log(LL_Debug, "Connecting to server %s:%d\n", ip.c_str(), port);

	if(CONFIG_GET()->server.clientIpBlockSupport.get() && ip == "127.0.0.1" &&
	   clientIp.type == StreamAddress::ST_SocketIpv4) {
		char clientAddress[INET6_ADDRSTRLEN];
		char computedBindAddress[INET6_ADDRSTRLEN];

		clientIp.getName(clientAddress, sizeof(clientAddress));

		StreamAddress bindAddress = clientIp;
		// ip is network order (big endian), replace the first ip part with 127 to make it a localhost IP
		bindAddress.rawAddress.ipv4.s_addr = 127 | (bindAddress.rawAddress.ipv4.s_addr & 0xFFFFFF00);
		bindAddress.getName(computedBindAddress, sizeof(computedBindAddress));
		localHostBindIp = computedBindAddress;

		log(LL_Debug,
		    "Binding client %s to localhost IP %s to handle GS ip blocking\n",
		    clientAddress,
		    localHostBindIp.c_str());
	} else {
		localHostBindIp.clear();
	}

	EncryptedSession<PacketSession>::connect(ip.c_str(), port);
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
		toClientBaseEndpoint->close();
		detachClient();
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
	if(sessionType == SessionType::AuthClient && packet->id == TS_AC_SERVER_LIST::packetID && clientSession &&
	   gameClientSessionManager) {
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
				    SessionType::GameClient,
				    listenIp,
				    baseListenPort,
				    server.server_ip,
				    server.server_port,
				    clientSession->getStream() ? clientSession->getStream()->getPacketLogger() : nullptr,
				    clientSession->getBanManager());
				if(!listenPort) {
					log(LL_Error,
					    "Failed to listen on %s for game server %s:%d\n",
					    listenIp.c_str(),
					    server.server_ip.c_str(),
					    server.server_port);
				} else {
					log(LL_Debug,
					    "Listening on %s:%d for game server %s:%d\n",
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
	} else if(sessionType == SessionType::GameClient && packet->id == TS_SC_URL_LIST::getId(getPacketVersion()) &&
	          clientSession && gameClientSessionManager) {
		TS_SC_URL_LIST urlListPkt;
		if(packet->process(urlListPkt, getPacketVersion())) {
			static const char* const GUILD_ICON_IP_KEY = "guild_icon_upload.ip";
			static const char* const GUILD_ICON_PORT_KEY = "guild_icon_upload.port";

			std::string listenIp = CONFIG_GET()->client.listener.listenIp.get();
			std::istringstream urlListStream(urlListPkt.url_list);
			std::string key;
			std::string value;
			std::string guildIconUploadIp;
			uint16_t guildIconUploadPort = 0;
			std::vector<std::string> urlList;

			if(!uploadBasePort)
				uploadBasePort = CONFIG_GET()->client.uploadBasePort.get();

			while(std::getline(urlListStream, key, '|') && std::getline(urlListStream, value, '|')) {
				if(key == GUILD_ICON_IP_KEY) {
					guildIconUploadIp = value;
				} else if(key == GUILD_ICON_PORT_KEY) {
					std::from_chars(value.data(), value.data() + value.size(), guildIconUploadPort);
				} else {
					urlList.push_back(key);
					urlList.push_back(value);
				}
			}

			if(!guildIconUploadIp.empty() && guildIconUploadPort != 0) {
				uint16_t listenPort = gameClientSessionManager->ensureListening(
				    SessionType::UploadClient,
				    listenIp,
				    uploadBasePort,
				    guildIconUploadIp,
				    guildIconUploadPort,
				    clientSession->getStream() ? clientSession->getStream()->getPacketLogger() : nullptr,
				    clientSession->getBanManager());
				if(!listenPort) {
					log(LL_Error,
					    "Failed to listen on %s for upload server %s:%d\n",
					    listenIp.c_str(),
					    guildIconUploadIp.c_str(),
					    guildIconUploadPort);
				} else {
					log(LL_Debug,
					    "Listening on %s:%d for upload server %s:%d\n",
					    listenIp.c_str(),
					    listenPort,
					    guildIconUploadIp.c_str(),
					    guildIconUploadPort);
				}

				urlList.push_back(GUILD_ICON_IP_KEY);
				urlList.push_back(CONFIG_GET()->client.gameExternalIp.get());
				urlList.push_back(GUILD_ICON_PORT_KEY);
				urlList.push_back(std::to_string(listenPort));

				// If not using random ports (ie: port 0), increment the base port for subsequent Upload server
				if(uploadBasePort)
					uploadBasePort++;

				std::ostringstream urlListOutput;
				std::copy(urlList.begin(), urlList.end(), std::ostream_iterator<std::string>(urlListOutput, "|"));
				urlListPkt.url_list = urlListOutput.str();

				// remove last '|'
				if(!urlListPkt.url_list.empty())
					urlListPkt.url_list.resize(urlListPkt.url_list.size() - 1);
			}
		}
		toClientBaseEndpoint->sendPacket(urlListPkt);
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
