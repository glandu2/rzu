#include "uv.h"
#include "Account.h"
#include "Authentication.h"
#include <string.h>
#include "EventLoop.h"
#include "RappelzLibInit.h"
#include <stdio.h>
#include "RappelzSocket.h"
#include "ConfigInfo.h"
#include "RappelzLibConfig.h"
#include "TimingFunctions.h"

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string &resultString);
void onAuthRetrieveServer(uv_timer_t* handle);
void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);
void onAuthClosed(IListener* instance, Authentication* auth);
void onAuthClosedWithFailure(IListener* instance, Authentication* auth);

std::vector<Account*> accounts;
std::vector<Authentication*> auths;
std::vector<uv_timer_t*> timers;
bool printDebug = false;
int connectionsStarted = 0;
int connectionsDone = 0;
int connectionTargetCount;
bool connectToGs = false;
int delay = 0;

static void init() {
	CFG_CREATE("ip", "127.0.0.1" /*"127.0.0.1"*/);
	CFG_CREATE("port", 4500);
	CFG_CREATE("account", "test");
	CFG_CREATE("count", 8);
	CFG_CREATE("targetcount", 3000);
	CFG_CREATE("password", "admin");
	CFG_CREATE("usersa", false);
	CFG_CREATE("printall", false);
	CFG_CREATE("connecttogs", false);
	CFG_CREATE("delay", 0);
	CFG_CREATE("idxoffset", 0);
	CFG_CREATE("usecperconnection", 0);
}

static std::string getIpForConnection(const std::string& originalIp, bool useLocalHost, int connection) {
	if(useLocalHost) {
		char buffer[20];
		sprintf(buffer, "127.0.0.%d", int(connection / 50000) + 1);

		return std::string(buffer);
	} else {
		return originalIp;
	}
}

int main(int argc, char *argv[])
{
	RappelzLibInit();
	init();
	ConfigInfo::get()->init(argc, argv);
	ConfigInfo::get()->dump();
	initTimer();

	int count = CFG_GET("count")->getInt();
	std::string accountNamePrefix = CFG_GET("account")->getString();
	std::string ip = CFG_GET("ip")->getString();
	int port = CFG_GET("port")->getInt();
	bool usersa = CFG_GET("usersa")->getBool();
	int idxoffset = CFG_GET("idxoffset")->getInt();
	bool useLocalHost = ip == "127.0.0.1";
	int usecBetweenConnection = CFG_GET("usecperconnection")->getInt();

	connectionTargetCount = CFG_GET("targetcount")->getInt();
	printDebug = CFG_GET("printall")->getBool();
	connectToGs = CFG_GET("connecttogs")->getBool();
	delay = CFG_GET("delay")->getInt();

	accounts.reserve(count);
	auths.reserve(count);
	if(delay)
		timers.reserve(count);
	for(int i = 0; i < count; i++) {
		const std::string accountName = (count > 1)? accountNamePrefix + std::to_string((long long)i + idxoffset) : accountNamePrefix;

		Account* account = new Account(accountName);
		Authentication* auth = new Authentication(getIpForConnection(ip, useLocalHost, i), usersa? Authentication::ACM_RSA_AES : Authentication::ACM_DES, port);
		auth->index = i;

		accounts.push_back(account);
		auths.push_back(auth);

		if(delay) {
			uv_timer_t* timer = new uv_timer_t;
			uv_timer_init(EventLoop::getLoop(), timer);
			timer->data = auth;
			timers.push_back(timer);
		}


	}

	fprintf(stderr, "Starting benchmark\n");

	if(usecBetweenConnection == 0)
		resetTimer();

	for(size_t i = 0; i < auths.size(); i++) {
		auths[i]->connect(accounts[i], CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
		connectionsStarted++;
		if(usecBetweenConnection)
			tfMicroSleep(usecBetweenConnection);
	}

	if(usecBetweenConnection != 0) {
		fprintf(stderr, "Connected %d connections at limited speed, continuing benchmark at full-speed (time counter begin now)\n", connectionsStarted);
		resetTimer();
	}

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	unsigned long long int duration = getTimerValue();

	fprintf(stderr, "%d connections in %llu usec => %f auth/sec\n", connectionsDone, duration, connectionsDone/((float)duration/1000000.0f));
}

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string& resultString) {
	if(printDebug || result != TS_RESULT_SUCCESS)
		fprintf(stderr, "%s: Auth result: %d (%s)\n", auth->getAccountName().c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
	if(result == TS_RESULT_SUCCESS) {
		if(delay == 0)
			auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(nullptr, &onServerList));
		else {
			uv_timer_start(timers[auth->index], &onAuthRetrieveServer, delay, 0);
		}
	} else {
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(nullptr, &onAuthClosedWithFailure));
	}
}

void onAuthRetrieveServer(uv_timer_t* handle) {
	Authentication* auth = (Authentication*) handle->data;

	auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(nullptr, &onServerList));
}

void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId) {
	if(printDebug) {
		fprintf(stderr, "%p Server list (last id: %d)\n", auth, lastSelectedServerId);
		for(size_t i = 0; i < servers->size(); i++) {
			fprintf(stderr, "%d: %20s at %16s:%d %d%% user ratio\n",
					servers->at(i).serverId,
					servers->at(i).serverName.c_str(),
					servers->at(i).serverIp.c_str(),
					servers->at(i).serverPort,
					servers->at(i).userRatio);
		}
	}
	if(connectToGs && servers->size() > 0)
		auth->selectServer(servers->at(0).serverId, Callback<Authentication::CallbackOnGameResult>(nullptr, &onGameResult));
	else
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(nullptr, &onAuthClosed));
}

void onAuthClosedWithFailure(IListener* instance, Authentication* auth) {
	if(printDebug)
		fprintf(stderr, "%p auth closed with failure\n", auth);
	//auth->connect(nullptr, CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
}

void onAuthClosed(IListener* instance, Authentication* auth) {
	if(printDebug)
		fprintf(stderr, "%p auth closed\n", auth);
	connectionsDone++;
	if(connectionsStarted < connectionTargetCount) {
		connectionsStarted++;
		auth->connect(nullptr, CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
	}
}

void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket) {
	fprintf(stderr, "login to GS result: %d\n", result);
	if(gameServerSocket) {
		connectionsDone++;
		gameServerSocket->close();
		if(connectionsStarted < connectionTargetCount) {
			connectionsStarted++;
			auth->connect(nullptr, CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
		}
	}
}
