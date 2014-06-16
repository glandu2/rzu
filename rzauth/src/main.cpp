#include "EventLoop.h"
#include "GlobalConfig.h"
#include "RappelzLibInit.h"
#include "RappelzLibConfig.h"
#include "CrashHandler.h"
#include "DbConnectionPool.h"

#include "AuthServer/DB_Account.h"
#include "AuthServer/DB_UpdateLastServerIdx.h"
#include "ServersManager.h"
#include "BanManager.h"
#include "SocketSession.h"

#include "AuthServer/ClientSession.h"
#include "AuthServer/GameServerSession.h"
#include "AuthServer/DB_Account.h"

#include "UploadServer/ClientSession.h"
#include "UploadServer/GameServerSession.h"
#include "UploadServer/IconServerSession.h"

#include "AdminServer/TelnetSession.h"


/* TODO for next version
 * valgrind: memcheck + callgrind + heap tool
 * optimize DbConnection selection by cached query
 */


/* TODO:
 *
 * DbBinding: cols: optionnal + was set bool
 * DbBinding: dynamic resize std::string column
 * 2 init functions names: one for init before config read, one for after
 * rename header guards
 * move init to have the same interface for all type of servers
 * move servers in DLL, the .exe would just host the one the user need to use
 *  -> separate config
 * warning: GS with auth in same exe: delay connectToAuth ?
 * manage more field in TS_AG_CLIENT_LOGIN (play time)
 */

/* Packet versionning:
 * Base class with all functions to access field but do nothing
 * Derived classes with packet fields + function to access them (no virtual stuff)
 * - Serialization:
 *   - << overload into socket: create buffer, fill it according to data in struct and version
 *      template to create buffer with size == packet or manual size + function to serialize into the buffer then to write it in socket
 *      use macro to generate struct content + serialization function at once ?
 *   - use get/set: set only when field exist in version, get default value is non existent in version
 *     -> generate files, no copy (serialization on the fly)
 *   - best wouldn't need template to use several version ...
 */

void runServers(Log* trafficLogger);
void showDebug(uv_timer_t*);

int main(int argc, char **argv) {
	RappelzLibInit();
	GlobalConfig::init();

	DbConnectionPool dbConnectionPool;

	if(AuthServer::DB_Account::init(&dbConnectionPool) == false)
		return 1;
	if(AuthServer::DB_UpdateLastServerIdx::init(&dbConnectionPool) == false)
		return 1;

	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(RappelzLibConfig::get()->log.enable,
				   RappelzLibConfig::get()->log.level,
				   RappelzLibConfig::get()->log.consoleLevel,
				   RappelzLibConfig::get()->log.dir,
				   RappelzLibConfig::get()->log.file,
				   RappelzLibConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	Log trafficLogger(CONFIG_GET()->trafficDump.enable,
					  CONFIG_GET()->trafficDump.level,
					  CONFIG_GET()->trafficDump.consoleLevel,
					  CONFIG_GET()->trafficDump.dir,
					  CONFIG_GET()->trafficDump.file,
					  RappelzLibConfig::get()->log.maxQueueSize);


	ConfigInfo::get()->dump();

	AuthServer::ClientSession::init(CONFIG_GET()->auth.client.desKey);

	CrashHandler::setDumpMode(CONFIG_GET()->admin.dumpMode);

	runServers(&trafficLogger);

	//Make valgrind happy
	AuthServer::ClientSession::deinit();
	AuthServer::DB_Account::deinit();
	EventLoop::getInstance()->deleteObjects();

	return 0;
}

void runServers(Log *trafficLogger) {
	ServersManager serverManager;
	BanManager banManager;

	RappelzServer<AuthServer::ClientSession> authClientServer(trafficLogger);
	RappelzServer<AuthServer::GameServerSession> authGameServer(trafficLogger);

	RappelzServer<UploadServer::ClientSession> uploadClientServer(trafficLogger);
	RappelzServer<UploadServer::IconServerSession> uploadIconServer;
	RappelzServer<UploadServer::GameServerSession> uploadGameServer(trafficLogger);

	RappelzServer<AdminServer::TelnetSession> adminTelnetServer;

	serverManager.addServer("auth.clients", &authClientServer,
							CONFIG_GET()->auth.client.listenIp,
							CONFIG_GET()->auth.client.port,
							CONFIG_GET()->auth.client.autoStart,
							&banManager);
	serverManager.addServer("auth.gameserver", &authGameServer,
							CONFIG_GET()->auth.game.listenIp,
							CONFIG_GET()->auth.game.port,
							CONFIG_GET()->auth.game.autoStart);

	serverManager.addServer("upload.clients", &uploadClientServer,
							CONFIG_GET()->upload.client.listenIp,
							CONFIG_GET()->upload.client.port,
							CONFIG_GET()->upload.client.autoStart,
							&banManager);
	serverManager.addServer("upload.iconserver", &uploadIconServer,
							CONFIG_GET()->upload.client.listenIp,
							CONFIG_GET()->upload.client.webPort,
							CONFIG_GET()->upload.client.autoStart,
							&banManager);
	serverManager.addServer("upload.gameserver", &uploadGameServer,
							CONFIG_GET()->upload.game.listenIp,
							CONFIG_GET()->upload.game.port,
							CONFIG_GET()->upload.game.autoStart);

	serverManager.addServer("admin.telnet", &adminTelnetServer,
							CONFIG_GET()->admin.telnet.listenIp,
							CONFIG_GET()->admin.telnet.port,
							CONFIG_GET()->admin.telnet.autoStart);

	serverManager.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void showDebug(uv_timer_t *) {
	char debugInfo[1000];
	strcpy(debugInfo, "----------------------------------\n");
	sprintf(debugInfo, "%s%lu socket Sessions\n", debugInfo, SocketSession::getObjectCount());
	sprintf(debugInfo, "%s%lu active connections\n", debugInfo, Socket::getObjectCount());
	puts(debugInfo);
}
