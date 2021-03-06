#pragma once

#include "Core/Timer.h"
#include "NetSession/ClientAuthSession.h"
#include <string>

struct BenchmarkConfig {
	uint16_t port;
	int delay;
	int recoDelay;
	int connectionsDone;
	int connectionsStarted;
	int connectionTargetCount;
	int epic;
};

class BenchmarkAuthSession : public ClientAuthSession {
public:
	BenchmarkAuthSession(BenchmarkConfig* config);

	void connect(const std::string& ip, const std::string& account, const std::string& password);

private:
	using ClientAuthSession::connect;

	virtual void onAuthDisconnected(bool causedByRemote);
	virtual void onAuthResult(TS_ResultCode result, const std::string& resultString);
	virtual void onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId);

	virtual void onGameDisconnected(bool causedByRemote);
	virtual void onGameResult(TS_ResultCode result);

	void onAuthDelayExpired();
	void onAuthRecoDelayExpired();

private:
	BenchmarkConfig* config;
	std::string ip;
	std::string account;
	std::string password;

	bool doReconnect;
	Timer<BenchmarkAuthSession> delayTimer;
	Timer<BenchmarkAuthSession> recoDelayTimer;
};

