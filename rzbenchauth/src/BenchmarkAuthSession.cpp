#include "BenchmarkAuthSession.h"
#include "Core/EventLoop.h"

BenchmarkAuthSession::BenchmarkAuthSession(BenchmarkConfig* config) : ClientAuthSession(nullptr)
{
	delayTimer.data = this;
	this->config = config;
	uv_timer_init(EventLoop::getLoop(), &delayTimer);
	recoDelayTimer.data = this;
	uv_timer_init(EventLoop::getLoop(), &recoDelayTimer);
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
			if(config->recoDelay > 0) {
				uv_timer_start(&recoDelayTimer, &onAuthRecoDelayExpired, config->recoDelay, 0);
			} else {
				ClientAuthSession::connect(ip, config->port, account, password, config->method, config->version);
			}
		}
	}
}

void BenchmarkAuthSession::onAuthRecoDelayExpired(uv_timer_t *timer) {
	BenchmarkAuthSession* thisInstance = (BenchmarkAuthSession*) timer->data;
	thisInstance->ClientAuthSession::connect(thisInstance->ip,
											 thisInstance->config->port,
											 thisInstance->account,
											 thisInstance->password,
											 thisInstance->config->method,
											 thisInstance->config->version);
}

void BenchmarkAuthSession::onAuthResult(TS_ResultCode result, const std::string& resultString) {
	if(result != TS_RESULT_SUCCESS) {
		log(LL_Warning, "%s: Auth failed result: %d (%s)\n", account.c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
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
	log(LL_Debug, "%s: Server list (last id: %d)\n", account.c_str(), lastSelectedServerId);
	for(size_t i = 0; i < servers.size(); i++) {
		log(LL_Debug, "%d: %20s at %16s:%d %d%% user ratio\n",
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
