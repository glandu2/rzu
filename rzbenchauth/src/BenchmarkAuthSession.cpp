#include "BenchmarkAuthSession.h"
#include "EventLoop.h"

BenchmarkAuthSession::BenchmarkAuthSession(BenchmarkConfig* config) : ClientAuthSession(nullptr)
{
	delayTimer.data = this;
	this->config = config;
	uv_timer_init(EventLoop::getLoop(), &delayTimer);
}

void BenchmarkAuthSession::connect(const std::string& ip, const std::string& account, const std::string& password) {
	this->ip = ip;
	this->account = account;
	this->password = password;

	doReconnect = false;

	config->connectionsStarted++;
	ClientAuthSession::connect(ip, config->port, account, password, config->method, config->version);
}

void BenchmarkAuthSession::onAuthDisconnected() {
	if(doReconnect) {
		doReconnect = false;
		config->connectionsDone++;
		if(config->connectionsStarted < config->connectionTargetCount) {
			config->connectionsStarted++;
			ClientAuthSession::connect(ip, config->port, account, password, config->method, config->version);
		}
	}
}

void BenchmarkAuthSession::onAuthResult(TS_ResultCode result, const std::string& resultString) {
	if(result != TS_RESULT_SUCCESS) {
		warn("%s: Auth failed result: %d (%s)\n", account.c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
		abortSession();
	} else {
		if(config->delay <= 0)
			retreiveServerList();
		else
			uv_timer_start(&delayTimer, &onAuthDelayExpired, config->delay, 0);
	}
}

void BenchmarkAuthSession::onAuthDelayExpired(uv_timer_t *timer) {
	BenchmarkAuthSession* thisInstance = (BenchmarkAuthSession*) timer->data;
	thisInstance->retreiveServerList();
}

void BenchmarkAuthSession::onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId) {
	debug("%s: Server list (last id: %d)\n", account.c_str(), lastSelectedServerId);
	for(size_t i = 0; i < servers.size(); i++) {
		debug("%d: %20s at %16s:%d %d%% user ratio\n",
				servers.at(i).serverId,
				servers.at(i).serverName.c_str(),
				servers.at(i).serverIp.c_str(),
				servers.at(i).serverPort,
				servers.at(i).userRatio);
	}

	doReconnect = true;
	abortSession();
}

void BenchmarkAuthSession::onGameDisconnected() {

}

void BenchmarkAuthSession::onGameResult(TS_ResultCode result) {

}
