#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include <string.h>
#include <algorithm>
#include "GlobalConfig.h"
#include "PrintfFormats.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_AC_SERVER_LIST.h"

#include "PacketFilter.h"

ClientSession::ClientSession()
  : serverSession(this), packetFilter(new PacketFilter)
{
}

ClientSession::~ClientSession() {
	delete packetFilter;
}

void ClientSession::onConnected() {
	info("Client connected, connecting server session\n");
	serverSession.connect();
	setDirtyObjectName();
}

void ClientSession::onDisconnected(bool causedByRemote) {
	info("Client disconnected, disconnecting server session\n");
	serverSession.closeSession();
}

void ClientSession::logPacket(bool outgoing, const TS_MESSAGE* msg) {
	trace("%s packet id: %5d, size: %d\n",
		  (outgoing)? "SERV->CLI" : "CLI->SERV",
		  msg->id,
		  int(msg->size - sizeof(TS_MESSAGE)));

	getStream()->packetLog(Log::LL_Debug, reinterpret_cast<const unsigned char*>(msg) + sizeof(TS_MESSAGE), (int)msg->size - sizeof(TS_MESSAGE),
			  "%s packet id: %5d, size: %d\n",
			  (outgoing)? "SERV->CLI" : "CLI->SERV",
			  msg->id,
			  int(msg->size - sizeof(TS_MESSAGE)));
}

void ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	if(packet->id != TS_CC_EVENT::packetID) {
		debug("Received packet id %d from client, forwarding to server\n", packet->id);
		if(packetFilter->onClientPacket(this, &serverSession, packet))
			serverSession.sendPacket(packet);
	}
}

void ClientSession::onServerPacketReceived(const TS_MESSAGE* packet) {
	if(packet->id == TS_AC_SERVER_LIST::packetID && CONFIG_GET()->client.authMode.get() == true) {
		char* buffer = (char*) malloc(packet->size);
		memcpy(buffer, packet, packet->size);

		TS_AC_SERVER_LIST* serverList = reinterpret_cast<TS_AC_SERVER_LIST*>(buffer);
		TS_AC_SERVER_LIST::TS_SERVER_INFO* serverDetailList = reinterpret_cast<TS_AC_SERVER_LIST::TS_SERVER_INFO*>(buffer+sizeof(TS_AC_SERVER_LIST));

		std::string filterIp = CONFIG_GET()->client.gameFilterIp.get();
		int filterPort = CONFIG_GET()->client.gameFilterPort.get();

		for(int i = 0; i < serverList->count; i++) {
			TS_AC_SERVER_LIST::TS_SERVER_INFO *currentServerInfo = &serverDetailList[i];
			strcpy(currentServerInfo->server_ip, filterIp.c_str());
			if(filterPort > 0)
				currentServerInfo->server_port = filterPort;
		}
		sendPacket(serverList);
		free(buffer);
	} else if(packet->id != TS_CC_EVENT::packetID) {
		debug("Received packet id %d from server, forwarding to client\n", packet->id);
		if(packetFilter->onServerPacket(this, &serverSession, packet))
			sendPacket(packet);
	}
}

void ClientSession::updateObjectName() {
	size_t streamNameSize;
	const char* streamName = getStream()->getObjectName(&streamNameSize);
	setObjectName(8 + (int)streamNameSize, "Client[%s]", streamName);
}
