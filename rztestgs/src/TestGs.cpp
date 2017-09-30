#include "TestGs.h"
#include "Core/EventLoop.h"
#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_CS_REPORT.h"
#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_SC_RESULT.h"
#include "PacketEnums.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

TestGs::TestGs(std::string host, uint16_t port, const std::string& reqStr) {
	this->host = host;
	this->port = port;
	this->connectedTimes = 0;
	this->reqStr = reqStr;
}

void TestGs::start() {
	log(LL_Info, "Connecting to GS %s:%d\n", host.c_str(), port);
	connect(host.c_str(), port);
}

void TestGs::stop() {
	abortSession();
}

EventChain<SocketSession> TestGs::onConnected() {
	TS_CS_VERSION versionMsg;

	// continue server move as we are connected now to game server
	versionMsg.szVersion = "201708120";
	sendPacket(versionMsg, EPIC_LATEST);

	TS_CS_ACCOUNT_WITH_AUTH loginInGameServerMsg;

	loginInGameServerMsg.account = "ddddddddogg";
	loginInGameServerMsg.one_time_key = 646541651;
	sendPacket(loginInGameServerMsg, EPIC_LATEST);

	TS_CS_REPORT reportMsg;
	reportMsg.report = "e";
	sendPacket(reportMsg, EPIC_LATEST);

	return PacketSession::onConnected();
}

EventChain<SocketSession> TestGs::onDisconnected(bool causedByRemote) {
	if(causedByRemote) {
		log(LL_Error, "Disconnected by server !\n");
	} else {
		log(LL_Info, "Disconnected\n");
		// start();
	}

	return PacketSession::onDisconnected(causedByRemote);
}

EventChain<PacketSession> TestGs::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_SC_RESULT::packetID: {
			TS_SC_RESULT resultMsg;
			bool deserializationOk = packet->process(resultMsg, EPIC_LATEST);

			if(deserializationOk && resultMsg.request_msg_id == TS_CS_ACCOUNT_WITH_AUTH::packetID) {
				log(LL_Info, "Account with auth returned %d / %d\n", resultMsg.result, resultMsg.value);
				close();
			} else {
				log(LL_Warning, "Failed to read TS_SC_RESULT: %d\n", packet->id);
			}
			break;
		}

		default: log(LL_Warning, "Unknown packet id %d\n", packet->id);
	}

	return PacketSession::onPacketReceived(packet);
}
