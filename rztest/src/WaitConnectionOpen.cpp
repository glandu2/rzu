#include "WaitConnectionOpen.h"

void WaitConnectionOpen::start(uint16_t port, int timeoutms, std::string host) {
	this->host = host;
	this->port = port;
	this->open = false;
	this->stop = false;

	this->timer.start(this, &WaitConnectionOpen::timerTimeout, timeoutms, 0);

	connect(host.c_str(), port);
}

EventChain<SocketSession> WaitConnectionOpen::onConnected() {
	log(LL_Debug, "Connected to %s:%d\n", host.c_str(), port);
	open = true;
	stop = true;
	timer.stop();
	closeSession();

	return SocketSession::onConnected();
}

EventChain<SocketSession> WaitConnectionOpen::onDisconnected(bool causedByRemote) {
	log(LL_Debug, "Disconnected\n");
	if(!stop) {
		retryTimer.start(this, &WaitConnectionOpen::retryTimerTimeout, 1000, 0);
	}
	return SocketSession::onDisconnected(causedByRemote);
}

void WaitConnectionOpen::retryTimerTimeout() {
	connect(host.c_str(), port);
}

EventChain<SocketSession> WaitConnectionOpen::onDataReceived() {
	if(getStream()) {
		getStream()->discardAll();
	}

	return SocketSession::onDataReceived();
}

void WaitConnectionOpen::timerTimeout() {
	stop = true;
}
