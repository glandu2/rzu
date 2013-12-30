#include "ClientInfo.h"
#include "ServerInfo.h"
#include "EventLoop.h"
#include "ConfigInfo.h"
#include "GlobalConfig.h"
#include "RappelzLibInit.h"

//socket->deleteLater in uv_check_t
void showDebug(uv_timer_t*, int);

int main(int argc, char **argv) {
//	uv_timer_t timer;
//	uv_timer_init(EventLoop::getLoop(), &timer);
//	uv_timer_start(&timer, &showDebug, 0, 3000);

	RappelzLibInit(argc, argv, &ConfigInfo::init);
	ConfigInfo::get()->dump(stdout);

	ClientInfo::startServer();
	ServerInfo::startServer();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

#include "Server.h"
#include "DB_Account.h"

void showDebug(uv_timer_t *, int) {
	char debugInfo[1000];
	strcpy(debugInfo, "----------------------------------\n");
	sprintf(debugInfo, "%s%lu Object\n", debugInfo, Object::getObjectCount());
	sprintf(debugInfo, "%s\t%lu EventLoop\n", debugInfo, EventLoop::getObjectCount());
	sprintf(debugInfo, "%s\t%lu Socket\n", debugInfo, Socket::getObjectCount());
	sprintf(debugInfo, "%s\t\t%lu EncryptedSocket\n", debugInfo, EncryptedSocket::getObjectCount());
	sprintf(debugInfo, "%s\t\t\t%lu RappelzSocket\n", debugInfo, RappelzSocket::getObjectCount());
	sprintf(debugInfo, "%s\t%lu Server\n", debugInfo, Server::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ClientInfo\n", debugInfo, ClientInfo::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ClientData\n", debugInfo, ClientData::getObjectCount());
	sprintf(debugInfo, "%s\t%lu DB_Account\n", debugInfo, DB_Account::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ServerInfo\n", debugInfo, ServerInfo::getObjectCount());
	printf(debugInfo);
}
