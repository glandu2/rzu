#include "PlayerCountMonitor.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_CA_VERSION.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

#define LOG_PREFIX "PlayerCountMonitor: "

PlayerCountMonitor::PlayerCountMonitor(std::string host, uint16_t port, int intervalms) : sock(uv_default_loop(), true) {
	this->host = host;
	this->port = port;

	printf("#msec since epoch, players connected number, process load\n");

	addInstance(sock.addPacketListener(TS_CA_VERSION::packetID, this, onPlayerCountReceived));
	addInstance(sock.addPacketListener(TS_CC_EVENT::packetID, this, onPlayerCountReceived));

	uv_timer_init(uv_default_loop(), &timer);
	timer.data = this;
	timeout = intervalms;
}

void PlayerCountMonitor::start() {
	 uv_timer_start(&timer, &updatePlayerNumber, 0, timeout);
}

void PlayerCountMonitor::stop() {
	 uv_timer_stop(&timer);
}

void PlayerCountMonitor::updatePlayerNumber(uv_timer_t* handle, int status) {
	PlayerCountMonitor* thisInstance = (PlayerCountMonitor*)handle->data;

	if(status < 0) {
		const char* errorString = uv_strerror(-status);
		fprintf(stderr, LOG_PREFIX"Socket: %s\n", errorString);
		return;
	}

	if(thisInstance->sock.getState() == Socket::UnconnectedState) {
		thisInstance->sock.connect(thisInstance->host, thisInstance->port);
	} else if(thisInstance->sock.getState() == Socket::ConnectingState) {
		thisInstance->sock.close();
		thisInstance->sock.connect(thisInstance->host, thisInstance->port);
		fprintf(stderr, LOG_PREFIX"Server connection timeout");
	} else {
		fprintf(stderr, LOG_PREFIX"Timer tick but server is not unconnected, timer is too fast ?");
	}
}

void PlayerCountMonitor::onPlayerCountReceived(void*, RappelzSocket* sock, const TS_MESSAGE* packetData) {

	switch(packetData->id) {
		case TS_SC_RESULT::packetID:
		{
			const TS_SC_RESULT* result = reinterpret_cast<const TS_SC_RESULT*>(packetData);
			switch(result->request_msg_id) {
				case TS_CA_VERSION::packetID:
					printf("%lu %d %d\n", time(NULL), result->value ^ 0xADADADAD, result->result);
					fflush(stdout);
					sock->close();
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
				strcpy(versionMsg.szVersion, "TEST");

				sock->sendPacket(&versionMsg);
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerConnectionLost) {
				fprintf(stderr, LOG_PREFIX"Disconnected from server !");
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerUnreachable) {
				fprintf(stderr, LOG_PREFIX"Unreachable server !");
			}
			break;
		}
	}

}
