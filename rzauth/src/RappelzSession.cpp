#include "RappelzSession.h"
#include "EventLoop.h"
#include "RappelzServer.h"

RappelzSession::RappelzSession(EncryptedSocket::Mode mode) : SocketSession(new RappelzSocket(EventLoop::getLoop(), mode)) {
}

void RappelzSession::addPacketsToListen(int packetsIdNum, const int packetsId[]) {
	for(int i = 0; i < packetsIdNum; i++) {
		getSocket()->addPacketListener(packetsId[i], this, &onDataReceived);
	}
}

void RappelzSession::onDataReceived(IListener* instance, RappelzSocket*, const TS_MESSAGE* packet) {
	static_cast<RappelzSession*>(instance)->onPacketReceived(packet);
}

void RappelzSession::onPacketReceived(const TS_MESSAGE*) {
}
