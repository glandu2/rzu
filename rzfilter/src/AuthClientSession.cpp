#include "AuthClientSession.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "GlobalConfig.h"
#include "GameClientSessionManager.h"

AuthClientSession::AuthClientSession(GameClientSessionManager* gameClientSessionManager)
    : ClientSession(true),
      gameClientSessionManager(gameClientSessionManager)
{
}

AuthClientSession::~AuthClientSession() {
}

std::string AuthClientSession::getServerIp()
{
	return CONFIG_GET()->server.ip.get();
}

uint16_t AuthClientSession::getServerPort()
{
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
			auto it = serverListPkt.servers.begin();
			auto itEnd = serverListPkt.servers.end();
			for(; it != itEnd; ++it) {
				TS_SERVER_INFO& server = *it;
				uint16_t listenPort = gameClientSessionManager->ensureListening(
				            listenIp,
				            server.server_ip,
				            server.server_port);
				if(!listenPort) {
					log(LL_Error, "Failed to listen on %s for server %s:%d\n",
					    listenIp.c_str(), server.server_ip.c_str(), server.server_port);
				} else {
					log(LL_Debug, "Listening on %s:%d for server %s:%d\n",
					    listenIp.c_str(), listenPort,
					    server.server_ip.c_str(), server.server_port);
				}
				server.server_ip = listenIp;
				server.server_port = listenPort;
			}
		}
		PacketSession::sendPacket(serverListPkt, getPacketVersion());
	} else {
		ClientSession::onServerPacketReceived(packet);
	}
}
