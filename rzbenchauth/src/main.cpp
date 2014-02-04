#include "uv.h"
#include "Account.h"
#include "Authentication.h"
#include <string.h>
#include "EventLoop.h"
#include "RappelzLibInit.h"
#include <stdio.h>
#include "RappelzSocket.h"

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const char* resultString);
void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);


int main(int argc, char *argv[])
{
	const char* host = "127.0.0.1";
	uint16_t port = 4500;

	if(argc >= 2)
		host = argv[1];

	if(argc >= 3)
		port = atoi(argv[2]);

	RappelzLibInit(argc, argv, nullptr);

	Account account("account");
	Authentication auth(host, Authentication::ACM_RSA_AES);
	auth.connect(&account, "password", Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const char* resultString) {
	fprintf(stderr, "Auth result: %d (%s)\n", result, resultString ? resultString : "no associated string");
	if(result == TS_RESULT_SUCCESS) {
		auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(nullptr, &onServerList));
	}
}
uint16_t serverId;
std::string serverName;
std::string serverScreenshotUrl;
std::string serverIp;
int32_t serverPort;
uint16_t userRatio;

void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId) {
	fprintf(stderr, "Server list (last id: %d)\n", lastSelectedServerId);
	for(size_t i = 0; i < servers->size(); i++) {
		fprintf(stderr, "%d: %20s at %16s:%d %d%% user ratio\n",
			   servers->at(i).serverId,
			   servers->at(i).serverName.c_str(),
			   servers->at(i).serverIp.c_str(),
			   servers->at(i).serverPort,
			   servers->at(i).userRatio);
	}
	auth->selectServer(lastSelectedServerId, Callback<Authentication::CallbackOnGameResult>(nullptr, &onGameResult));
}

void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket) {
	fprintf(stderr, "login to GS result: %d\n", result);
	if(result == 1)
		printf("wtf");
	if(gameServerSocket)
		gameServerSocket->close();
}
