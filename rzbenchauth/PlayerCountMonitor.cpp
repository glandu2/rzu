#include "PlayerCountMonitor.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_CA_VERSION.h"
#include <QDateTime>
#include <stdio.h>

#define LOG_PREFIX "PlayerCountMonitor: "

PlayerCountMonitor::PlayerCountMonitor(std::string host, quint16 port, int intervalms) {
	this->playerNumber = -1;
	this->host = host;
	this->port = port;

	printf("#msec since epoch, players connected number, process load\n");

	addInstance(server.addPacketListener(Server::ST_Auth, TS_CA_VERSION::packetID, this, onPlayerCountReceived));
	addInstance(server.addPacketListener(Server::ST_Auth, TS_CC_EVENT::packetID, this, onPlayerCountReceived));

	timer.setInterval(intervalms);
	timer.setSingleShot(false);
	connect(&timer, SIGNAL(timeout()), this, SLOT(updatePlayerNumber()));

	this->server.setServerFarm(host, port);
}

void PlayerCountMonitor::updatePlayerNumber() {
	if(server.getState() == Server::SS_NotConnected) {
		server.connectToAuth();
	} else if(server.getState() == Server::SS_ConnectingToAuth) {
		server.close();
		server.connectToAuth();
		qDebug(LOG_PREFIX"Server connection timeout");
	} else {
		qDebug(LOG_PREFIX"Timer tick but server is not unconnected, timer is too fast ?");
	}
}

void PlayerCountMonitor::onPlayerCountReceived(void* instance, Server* server, const TS_MESSAGE* packetData) {
	PlayerCountMonitor *thisInstance = static_cast<PlayerCountMonitor*>(instance);
	switch(packetData->id) {
		case TS_SC_RESULT::packetID:
		{
			const TS_SC_RESULT* result = reinterpret_cast<const TS_SC_RESULT*>(packetData);
			switch(result->request_msg_id) {
				case TS_CA_VERSION::packetID:
					thisInstance->playerNumber = result->value ^ 0xADADADAD;
					thisInstance->processLoad = result->result;
					server->close();
					thisInstance->playerNumberUpdated();
					break;
			}
			break;
		}

		case TS_CC_EVENT::packetID:
		{
			const TS_CC_EVENT* eventMsg = reinterpret_cast<const TS_CC_EVENT*>(packetData);
			if(eventMsg->event == TS_CC_EVENT::CE_ServerConnected) {
				TS_CA_VERSION versionMsg;
				TS_MESSAGE::initMessage<TS_CA_VERSION>(&versionMsg);
				qstrcpy(versionMsg.szVersion, "TEST");

				server->sendPacket(&versionMsg, Server::ST_Auth);
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerConnectionLost) {
				qDebug(LOG_PREFIX"Disconnected from server !");
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerUnreachable) {
				qDebug(LOG_PREFIX"Unreachable server !");
				//timer.stop();
			}
			break;
		}
	}

}

void PlayerCountMonitor::playerNumberUpdated() {
	printf("%llu %d %d\n", (QDateTime::currentMSecsSinceEpoch()+500)/1000, this->playerNumber, this->processLoad);
	fflush(stdout);
}
