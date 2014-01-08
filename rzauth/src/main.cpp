#include "EventLoop.h"
#include "GlobalConfig.h"
#include "RappelzLibInit.h"

#include "AuthServer/ClientSession.h"
#include "AuthServer/GameServerSession.h"
#include "AuthServer/DB_Account.h"

#include "UploadServer/ClientSession.h"
#include "UploadServer/GameServerSession.h"
#include "UploadServer/IconServerSession.h"

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

	AuthServer::ClientSession::startServer();
	AuthServer::GameServerSession::startServer();
	UploadServer::ClientSession::startServer();
	UploadServer::GameServerSession::startServer();
	UploadServer::IconServerSession::startServer();

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
	sprintf(debugInfo, "%s\t%lu ClientInfo\n", debugInfo, AuthServer::ClientSession::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ClientData\n", debugInfo, AuthServer::ClientData::getObjectCount());
	sprintf(debugInfo, "%s\t%lu DB_Account\n", debugInfo, AuthServer::DB_Account::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ServerInfo\n", debugInfo, AuthServer::GameServerSession::getObjectCount());
	printf(debugInfo);
}
