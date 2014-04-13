#include "EventLoop.h"
#include "GlobalConfig.h"
#include "RappelzLibInit.h"

#include "AuthServer/DB_Account.h"
#include "ServersManager.h"
#include "SocketSession.h"


/* TODO
 * Log packets
 * Telnet for commands (like stop which would help to have a correct valgrind output):
 *  - stop - stop the server
 *  - stats - show stats of the server (player count, active connections, connected GS, ...)
 *  - set - set variable value
 *  - get - get variable value
 *  - dump - dump variables
 */

void runServers();
void showDebug(uv_timer_t*);

int main(int argc, char **argv) {
//	uv_timer_t timer;
//	uv_timer_init(EventLoop::getLoop(), &timer);
//	uv_timer_start(&timer, &showDebug, 0, 3000);

	RappelzLibInit(argc, argv, &GlobalConfig::init);
	if(AuthServer::DB_Account::init() == false) {
		return -1;
	}

	runServers();

	//Make valgrind happy
	EventLoop::getInstance()->deleteObjects();

	return 0;
}

void runServers() {
	ServersManager serverManager;
	serverManager.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void showDebug(uv_timer_t *) {
	char debugInfo[1000];
	strcpy(debugInfo, "----------------------------------\n");
	sprintf(debugInfo, "%s%lu socket Sessions\n", debugInfo, SocketSession::getObjectCount());
	sprintf(debugInfo, "%sstats.connections = %d\n", debugInfo, CONFIG_GET()->stats.connectionCount.get());
	sprintf(debugInfo, "%sstats.disconnections = %d\n", debugInfo, CONFIG_GET()->stats.disconnectionCount.get());
	puts(debugInfo);
}
