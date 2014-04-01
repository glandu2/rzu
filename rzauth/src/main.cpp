#include "EventLoop.h"
#include "GlobalConfig.h"
#include "RappelzLibInit.h"

#include "AuthServer/ClientSession.h"
#include "AuthServer/GameServerSession.h"
#include "AuthServer/DB_Account.h"

#include "UploadServer/ClientSession.h"
#include "UploadServer/GameServerSession.h"
#include "UploadServer/IconServerSession.h"

#include "RappelzServer.h"
#include "BanManager.h"


/* TODO
 * Log packets
 * disable rsa
 */

void showDebug(uv_timer_t*, int);

int main(int argc, char **argv) {
//	uv_timer_t timer;
//	uv_timer_init(EventLoop::getLoop(), &timer);
//	uv_timer_start(&timer, &showDebug, 0, 3000);

	RappelzLibInit(argc, argv, &GlobalConfig::init);
	if(AuthServer::DB_Account::init() == false) {
		return -1;
	}

	BanManager banManager;
	banManager.loadFile();

	RappelzServer<AuthServer::ClientSession> authClientServer;
	RappelzServer<AuthServer::GameServerSession> authGameServer;

	authClientServer.startServer(CONFIG_GET()->auth.client.listenIp, CONFIG_GET()->auth.client.port, &banManager);
	authGameServer.startServer(CONFIG_GET()->auth.game.listenIp, CONFIG_GET()->auth.game.port);

	RappelzServer<UploadServer::ClientSession> uploadClientServer;
	RappelzServer<UploadServer::IconServerSession> uploadIconServer;
	RappelzServer<UploadServer::GameServerSession> uploadGameServer;

	uploadClientServer.startServer(CONFIG_GET()->upload.client.listenIp, CONFIG_GET()->upload.client.port, &banManager);
	uploadIconServer.startServer(CONFIG_GET()->upload.client.listenIp, CONFIG_GET()->upload.client.webPort, &banManager);
	uploadGameServer.startServer(CONFIG_GET()->upload.game.listenIp, CONFIG_GET()->upload.game.port);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void showDebug(uv_timer_t *, int) {
	char debugInfo[1000];
	strcpy(debugInfo, "----------------------------------\n");
	sprintf(debugInfo, "%s%lu socket Sessions\n", debugInfo, SocketSession::getObjectCount());
	puts(debugInfo);
}
