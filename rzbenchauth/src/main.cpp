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
void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);
void onAuthClosed(IListener* instance, Authentication* auth);
void onAuthClosedWithFailure(IListener* instance, Authentication* auth);

std::vector<Account*> accounts;
std::vector<Authentication*> auths;
bool printDebug = false;
int connectionsStarted = 0;
int connectionsDone = 0;
int connectionTargetCount;

static void init() {
	CFG_CREATE("ip", "127.0.0.1" /*"127.0.0.1"*/);
	CFG_CREATE("port", 4500);
	CFG_CREATE("account", "test");
	CFG_CREATE("count", 8);
	CFG_CREATE("targetcount", 3000);
	CFG_CREATE("password", "admin");
	CFG_CREATE("usersa", false);
	CFG_CREATE("printall", false);
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
	connectionTargetCount = CFG_GET("targetcount")->getInt();
	bool usersa = CFG_GET("usersa")->getBool();
	printDebug = CFG_GET("printall")->getBool();

	if(count > 1) {
		for(int i = 0; i < count; i++) {
			Account* account = new Account(accountNamePrefix + std::to_string((long long)i));
			Authentication* auth = new Authentication(ip, usersa? Authentication::ACM_RSA_AES : Authentication::ACM_DES, port);

			accounts.push_back(account);
			auths.push_back(auth);

			connectionsStarted++;
		}
	} else {
		Account* account = new Account(accountNamePrefix);
		Authentication* auth = new Authentication(ip, usersa? Authentication::ACM_RSA_AES : Authentication::ACM_DES, port);

		accounts.push_back(account);
		auths.push_back(auth);

		connectionsStarted++;
	}

	resetTimer();
	for(int i = 0; i < auths.size(); i++) {
		auths[i]->connect(accounts[i], CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
	}


	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	unsigned long long int duration = getTimerValue();

	fprintf(stderr, "%d connections in %llu usec => %f auth/sec\n", connectionsDone, duration, connectionsDone/((float)duration/1000000.0f));
}

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string& resultString) {
	if(printDebug || result != TS_RESULT_SUCCESS)
		fprintf(stderr, "%s: Auth result: %d (%s)\n", auth->getAccountName().c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
	if(result == TS_RESULT_SUCCESS) {
		auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(nullptr, &onServerList));
	} else {
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(nullptr, &onAuthClosedWithFailure));
	}
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
	/*if(servers->size() > 0)
		auth->selectServer(servers->at(0).serverId, Callback<Authentication::CallbackOnGameResult>(nullptr, &onGameResult));
	else*/
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
		gameServerSocket->close();
		if(connectionsStarted < connectionTargetCount) {
			connectionsStarted++;
			auth->connect(nullptr, CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
		}
	}
}
