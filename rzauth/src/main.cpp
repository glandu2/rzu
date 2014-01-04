#include "EventLoop.h"
#include "GlobalConfig.h"
#include "RappelzLibInit.h"

#include "AuthServer/ClientInfo.h"
#include "AuthServer/ServerInfo.h"
#include "AuthServer/DB_Account.h"

#include "UploadServer/ClientInfo.h"
#include "UploadServer/GameServerInfo.h"
#include "UploadServer/GuildIconServer.h"

//socket->deleteLater in uv_check_t
void showDebug(uv_timer_t*, int);

int main(int argc, char **argv) {
//	uv_timer_t timer;
//	uv_timer_init(EventLoop::getLoop(), &timer);
//	uv_timer_start(&timer, &showDebug, 0, 3000);

	RappelzLibInit(argc, argv, &GlobalConfig::init);
	if(AuthServer::DB_Account::init() == false) {
		return -1;
	}

	AuthServer::ClientInfo::startServer();
	AuthServer::ServerInfo::startServer();
	UploadServer::ClientInfo::startServer();
	UploadServer::GameServerInfo::startServer();
	UploadServer::GuildIconServer::startServer();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void showDebug(uv_timer_t *, int) {
	char debugInfo[1000];
	strcpy(debugInfo, "----------------------------------\n");
	sprintf(debugInfo, "%s%lu Object\n", debugInfo, Object::getObjectCount());
	sprintf(debugInfo, "%s\t%lu EventLoop\n", debugInfo, EventLoop::getObjectCount());
	sprintf(debugInfo, "%s\t%lu Socket\n", debugInfo, Socket::getObjectCount());
	sprintf(debugInfo, "%s\t\t%lu EncryptedSocket\n", debugInfo, EncryptedSocket::getObjectCount());
	sprintf(debugInfo, "%s\t\t\t%lu RappelzSocket\n", debugInfo, RappelzSocket::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ClientInfo\n", debugInfo, AuthServer::ClientInfo::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ClientData\n", debugInfo, AuthServer::ClientData::getObjectCount());
	sprintf(debugInfo, "%s\t%lu DB_Account\n", debugInfo, AuthServer::DB_Account::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ServerInfo\n", debugInfo, AuthServer::ServerInfo::getObjectCount());
	printf(debugInfo);
}
