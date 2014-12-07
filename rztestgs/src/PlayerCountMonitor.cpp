#include "PlayerCountMonitor.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_CA_VERSION.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "EventLoop.h"

PlayerCountMonitor::PlayerCountMonitor(std::string host, uint16_t port, const std::string &reqStr, int intervalms) : sock(EventLoop::getLoop(), EncryptedSocket::Encrypted) {
	this->host = host;
	this->port = port;
	this->connectedTimes = 0;
	this->reqStr = reqStr;

	printf("#msec since epoch, players connected number, process load\n");

	sock.addPacketListener(TS_CA_VERSION::packetID, this, onPlayerCountReceived);
	sock.addPacketListener(TS_CC_EVENT::packetID, this, onPlayerCountReceived);

	uv_timer_init(EventLoop::getLoop(), &timer);
	timer.data = this;
	timeout = intervalms;
}

void PlayerCountMonitor::start() {
	 uv_timer_start(&timer, &updatePlayerNumber, 0, timeout);
}

void PlayerCountMonitor::stop() {
	 uv_timer_stop(&timer);
}

void PlayerCountMonitor::updatePlayerNumber(uv_timer_t* handle) {
	PlayerCountMonitor* thisInstance = (PlayerCountMonitor*)handle->data;

	if(thisInstance->sock.getState() == Stream::UnconnectedState) {
		thisInstance->connectedTimes = 0;
		thisInstance->sock.connect(thisInstance->host, thisInstance->port);
	} else if(thisInstance->sock.getState() == Stream::ConnectingState) {
		thisInstance->sock.close();
		thisInstance->error("Server connection timeout\n");
	} else {
		if(thisInstance->sock.getState() == Stream::ConnectedState) {
			thisInstance->connectedTimes++;
		}

		//we see more than 5 times the socket connected, it should disconnect, close it in case of hanged state
		if(thisInstance->connectedTimes >= 5) {
			thisInstance->sock.abort();
			thisInstance->connectedTimes = 0;
		}
		thisInstance->error("Timer tick but server is not unconnected, timer is too fast ?\n");
	}
}

void PlayerCountMonitor::onPlayerCountReceived(IListener* instance, RappelzSocket* sock, const TS_MESSAGE* packetData) {
	PlayerCountMonitor* thisInstance = (PlayerCountMonitor*)instance;

	switch(packetData->id) {
		case TS_SC_RESULT::packetID:
		{
			const TS_SC_RESULT* result = reinterpret_cast<const TS_SC_RESULT*>(packetData);
			switch(result->request_msg_id) {
				case TS_CA_VERSION::packetID:
                    printf("%lu %d %d\n", (unsigned long)time(NULL), result->value ^ 0xADADADAD, result->result);
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
#ifndef NDEBUG
				memset(versionMsg.szVersion, 0, sizeof(versionMsg.szVersion));
#endif
				strcpy(versionMsg.szVersion, thisInstance->reqStr.c_str());

				sock->sendPacket(&versionMsg);
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerConnectionLost) {
				thisInstance->error("Disconnected from server !\n");
			}
			if(eventMsg->event == TS_CC_EVENT::CE_ServerUnreachable) {
				thisInstance->error("Unreachable server !\n");
			}
			break;
		}
	}

}
