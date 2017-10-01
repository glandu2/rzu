#include "AuthClientSession.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "GameClientSessionManager.h"
#include "GlobalConfig.h"
#include <algorithm>
#include <vector>

AuthClientSession::AuthClientSession(GameClientSessionManager* gameClientSessionManager)
    : ClientSession(true), gameClientSessionManager(gameClientSessionManager) {}

AuthClientSession::~AuthClientSession() {}

std::string AuthClientSession::getServerIp() {
	return CONFIG_GET()->server.ip.get();
}

uint16_t AuthClientSession::getServerPort() {
	return CONFIG_GET()->server.port.get();
}

EventChain<PacketSession> AuthClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	return ClientSession::onPacketReceived(packet);
}

void AuthClientSession::onServerPacketReceived(const TS_MESSAGE* packet) {
	if(packet->id == TS_AC_SERVER_LIST::packetID) {
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
				    this->getStream() ? this->getStream()->getPacketLogger() : nullptr);
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
		PacketSession::sendPacket(serverListPkt, getPacketVersion());
	} else {
		ClientSession::onServerPacketReceived(packet);
	}
}
