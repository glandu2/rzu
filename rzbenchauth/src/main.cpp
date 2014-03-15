#include "uv.h"
#include "Account.h"
#include "Authentication.h"
#include <string.h>
#include "EventLoop.h"
#include "RappelzLibInit.h"
#include <stdio.h>
#include "RappelzSocket.h"
#include "ConfigInfo.h"

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string &resultString);
void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);
void onAuthClosed(IListener* instance, Authentication* auth);

Account *globalAccount1;
Account *globalAccount2;

static void init() {
	CFG("ip", "127.0.0.1");
	CFG("port", 4500);
	CFG("account", "admin");
	CFG("account2", "adideut");
	CFG("password", "admin");
}

int main(int argc, char *argv[])
{
	RappelzLibInit(argc, argv, &init);

	Account account(CFG("account", "admin").get());
	Account account2(CFG("account2", "adideut").get());
	globalAccount1 = &account;
	globalAccount2 = &account2;

	Authentication auth(CFG("ip", "127.0.0.1").get(), Authentication::ACM_DES, CFG("port", 4500).get());
	Authentication auth2(CFG("ip", "127.0.0.1").get(), Authentication::ACM_DES, CFG("port", 4500).get());
	auth.connect(globalAccount1, CFG("password", "admin").get(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
	auth2.connect(globalAccount2, CFG("password", "admin").get(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string& resultString) {
	fprintf(stderr, "Auth result: %d (%s)\n", result, resultString.empty() ? "no associated string" : resultString.c_str());
	if(result == TS_RESULT_SUCCESS) {
		auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(nullptr, &onServerList));
	} else {
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>());
	}
}

void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId) {
	fprintf(stderr, "%p Server list (last id: %d)\n", auth, lastSelectedServerId);
	for(size_t i = 0; i < servers->size(); i++) {
		fprintf(stderr, "%d: %20s at %16s:%d %d%% user ratio\n",
			   servers->at(i).serverId,
			   servers->at(i).serverName.c_str(),
			   servers->at(i).serverIp.c_str(),
			   servers->at(i).serverPort,
			   servers->at(i).userRatio);
	}
	if(servers->size() > 0)
		auth->selectServer(lastSelectedServerId, Callback<Authentication::CallbackOnGameResult>(nullptr, &onGameResult));
	else
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(nullptr, &onAuthClosed));
}

void onAuthClosed(IListener* instance, Authentication* auth) {
	static int i = 0;
	i++;
	if(i < 1000)
		auth->connect(nullptr, CFG("password", "admin").get(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
}

void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket) {
	fprintf(stderr, "login to GS result: %d\n", result);
	if(gameServerSocket) {
		gameServerSocket->close();
		auth->connect(nullptr, CFG("password", "admin").get(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
	}
}
