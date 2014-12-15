#include "PlayerCountMonitor.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_CA_VERSION.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "EventLoop.h"

PlayerCountMonitor::PlayerCountMonitor(std::string host, uint16_t port, const std::string &reqStr, int intervalms) {
	this->host = host;
	this->port = port;
	this->connectedTimes = 0;
	this->reqStr = reqStr;

	printf("#msec since epoch, players connected number, process load\n");

	uv_timer_init(EventLoop::getLoop(), &timer);
	timeout = intervalms;
}

void PlayerCountMonitor::start() {
	uv_timer_start(&timer, &updatePlayerNumberStatic, 0, timeout);
	timer.data = this;
}

void PlayerCountMonitor::stop() {
	uv_timer_stop(&timer);
}

void PlayerCountMonitor::updatePlayerNumberStatic(uv_timer_t* handle) { static_cast<PlayerCountMonitor*>(handle->data)->updatePlayerNumber(); }
void PlayerCountMonitor::updatePlayerNumber() {
	if(getStream() == nullptr || getStream()->getState() == Stream::UnconnectedState) {
		connectedTimes = 0;
		connect(host.c_str(), port);
	} else if(getStream()->getState() == Stream::ConnectingState) {
		getStream()->close();
		error("Server connection timeout\n");
	} else {
		if(getStream()->getState() == Stream::ConnectedState) {
			connectedTimes++;
		}

		//we see more than 5 times the socket connected, it should disconnect, close it in case of hanged state
		if(connectedTimes >= 5) {
			abortSession();
			connectedTimes = 0;
		}
		error("Timer tick but server is not unconnected, timer is too fast ?\n");
	}
}

void PlayerCountMonitor::onPacketReceived(const TS_MESSAGE* packetData) {
	switch(packetData->id) {
		case TS_SC_RESULT::packetID:
		{
			const TS_SC_RESULT* result = reinterpret_cast<const TS_SC_RESULT*>(packetData);
			switch(result->request_msg_id) {
				case TS_CA_VERSION::packetID:
                    printf("%lu %d %d\n", (unsigned long)time(NULL), result->value ^ 0xADADADAD, result->result);
					fflush(stdout);
					close();
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
#ifndef NDEBUG
				memset(versionMsg.szVersion, 0, sizeof(versionMsg.szVersion));
#endif
				strcpy(versionMsg.szVersion, reqStr.c_str());

				sendPacket(&versionMsg);
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerConnectionLost) {
				error("Disconnected from server !\n");
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerUnreachable) {
				error("Unreachable server !\n");
			}
			break;
		}
	}

}
