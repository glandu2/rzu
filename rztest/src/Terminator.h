#pragma once

#include "Core/Timer.h"
#include "Extern.h"
#include "NetSession/SocketSession.h"

class RZTEST_EXTERN Terminator : public SocketSession {
	DECLARE_CLASS(Terminator)
public:
	void start(std::string host, uint16_t port, std::string command = "terminate", int timeoutMs = 0);

protected:
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<SocketSession> onDataReceived();

	void onTimeout();

private:
	std::string host;
	uint16_t port;
	std::string command;
	Timer<Terminator> timeoutTimer;
};

