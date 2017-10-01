#ifndef WAITCONNECTIONOPEN_H
#define WAITCONNECTIONOPEN_H

#include "Core/Timer.h"
#include "Extern.h"
#include "NetSession/SocketSession.h"

class RZTEST_EXTERN WaitConnectionOpen : public SocketSession {
	DECLARE_CLASS(WaitConnectionOpen)
public:
	void start(uint16_t port, int timeoutms, std::string host = "127.0.0.1");
	bool isOpen() { return open; }

protected:
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<SocketSession> onDataReceived();

	void timerTimeout();
	void retryTimerTimeout();

private:
	std::string host;
	uint16_t port;
	Timer<WaitConnectionOpen> timer;
	Timer<WaitConnectionOpen> retryTimer;
	bool open;
	bool stop;
};

#endif  // WAITCONNECTIONOPEN_H
