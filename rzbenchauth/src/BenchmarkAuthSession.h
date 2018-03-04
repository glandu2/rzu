#ifndef BENCHMARKAUTHSESSION_H
#define BENCHMARKAUTHSESSION_H

#include "Core/Timer.h"
#include "NetSession/ClientAuthSession.h"
#include <string>

struct BenchmarkConfig {
	uint16_t port;
	ClientAuthSession::AuthCipherMethod method;
	int delay;
	int recoDelay;
	int connectionsDone;
	int connectionsStarted;
	int connectionTargetCount;
};

class BenchmarkAuthSession : public ClientAuthSession {
public:
	BenchmarkAuthSession(BenchmarkConfig* config);

	void connect(const std::string& ip, const std::string& account, const std::string& password);

private:
	using ClientAuthSession::connect;

	virtual void onAuthDisconnected();
	virtual void onAuthResult(TS_ResultCode result, const std::string& resultString);
	virtual void onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId);

	virtual void onGameDisconnected();
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

#endif  // BENCHMARKAUTHSESSION_H
