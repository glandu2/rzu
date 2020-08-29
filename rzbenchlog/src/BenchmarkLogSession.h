#pragma once

#include "Core/EventChain.h"
#include "Core/Timer.h"
#include "NetSession/SocketSession.h"

struct BenchmarkConfig {
	int delay;
	int packetPerSalve;
	int packetSent;
	int packetTargetCount;
};

class BenchmarkLogSession : public SocketSession {
public:
	BenchmarkLogSession(BenchmarkConfig* config);

private:
	EventChain<SocketSession> onConnected();

	void sendPackets();

private:
	BenchmarkConfig* config;
	char buffer[1024];

	Timer<BenchmarkLogSession> delayTimer;
};

