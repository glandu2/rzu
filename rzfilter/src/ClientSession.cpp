#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include <string.h>
#include <algorithm>
#include "GlobalConfig.h"
#include "Core/PrintfFormats.h"
#include "PacketStructsName.h"

#include "PacketEnums.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_AC_SELECT_SERVER.h"
#include "Packet/PacketEpics.h"

#include "FilterManager.h"

ClientSession::ClientSession()
  : serverSession(this), packetFilter(new FilterManager), version(CONFIG_GET()->client.epic.get())
{
}

ClientSession::~ClientSession() {
	delete packetFilter;
}

EventChain<SocketSession> ClientSession::onConnected() {
	log(LL_Info, "Client connected, connecting server session\n");
	serverSession.connect();
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
	const char* packetName = getPacketName(msg->id, CONFIG_GET()->client.authMode.get());

	log(LL_Debug, "%s packet id: %5d, name %s, size: %d\n",
		(outgoing)? "SERV->CLI" : "CLI->SERV",
		msg->id,
		packetName,
		int(msg->size - sizeof(TS_MESSAGE)));

	getStream()->packetLog(Object::LL_Debug, reinterpret_cast<const unsigned char*>(msg) + sizeof(TS_MESSAGE), (int)msg->size - sizeof(TS_MESSAGE),
						   "%s packet id: %5d, name %s, size: %d\n",
						   (outgoing)? "SERV->CLI" : "CLI->SERV",
						   msg->id,
						   packetName,
						   int(msg->size - sizeof(TS_MESSAGE)));
}

EventChain<PacketSession> ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	//log(LL_Debug, "Received packet id %d from client, forwarding to server\n", packet->id);
	if(packetFilter->onClientPacket(this, &serverSession, packet))
		serverSession.sendPacket(packet);

	return PacketSession::onPacketReceived(packet);
}

void ClientSession::onServerPacketReceived(const TS_MESSAGE* packet) {
	if(packet->id == TS_AC_SERVER_LIST::packetID && CONFIG_GET()->client.authMode.get() == true) {
		MessageBuffer buffer(packet, packet->size, version);
		TS_AC_SERVER_LIST serverList;

		serverList.deserialize(&buffer);
		if(!buffer.checkFinalSize()) {
			return;
		}

		std::vector<TS_SERVER_INFO>& serverDetailList = serverList.servers;

		std::string filterIp = CONFIG_GET()->client.gameFilterIp.get();
		int filterPort = CONFIG_GET()->client.gameFilterPort.get();

		for(size_t i = 0; i < serverDetailList.size(); i++) {
			TS_SERVER_INFO& currentServerInfo = serverDetailList[i];
			strcpy(currentServerInfo.server_ip, filterIp.c_str());
			if(filterPort > 0)
				currentServerInfo.server_port = filterPort;
		}
		PacketSession::sendPacket(serverList, version);
	} else {
		//log(LL_Debug, "Received packet id %d from server, forwarding to client\n", packet->id);
		if(packetFilter->onServerPacket(this, &serverSession, packet))
			sendPacket(packet);
	}
}

void ClientSession::updateObjectName() {
	size_t streamNameSize;
	const char* streamName = getStream()->getObjectName(&streamNameSize);
	setObjectName(8 + (int)streamNameSize, "Client[%s]", streamName);
}
