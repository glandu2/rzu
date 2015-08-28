#ifndef BenchmarkLogSession_H
#define BenchmarkLogSession_H

#include "NetSession/SocketSession.h"
#include "LS_11N4S.h"
#include <string>

struct BenchmarkConfig {
	int delay;
	int packetPerSalve;
	int packetSent;
	int packetTargetCount;
};

class BenchmarkLogSession : public SocketSession
{
public:
	BenchmarkLogSession(BenchmarkConfig* config);

private:
	void onConnected();
	void onDisconnected(bool causedByRemote);

	void sendPackets();
	static void sendPacketsStatic(uv_timer_t* timer);

private:
	BenchmarkConfig* config;
	char buffer[1024];

	uv_timer_t delayTimer;
};

#endif // BenchmarkLogSession_H
