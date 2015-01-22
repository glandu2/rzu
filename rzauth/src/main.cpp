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

#include "AdminServer/AdminInterface.h"
#include "AuthServer/BillingInterface.h"

/* TODO for next version
 */

/* TODO:
 *
 * flood prevention:
 *	max connections / ip  (limit flooding ips)
 *  max total connections (avoid too much memory usage) (no accept until connection count < max)
 *
 * Allow to change gameserver infos via admin interface (like external ip, index (to dyn hide/unhide from players))
 * Avoid a buffer in Socket (like uv_tcp_t)
 * Config: init config per classes (no more globalconfig.h) => avoid unused config declaration (benchmark does not use core.log.*)
 * DbBinding: cols: optionnal + "was set" bool
 * DbBinding: dynamic resize std::string column
 * 2 init functions names: one for init before config read, one for after (ex: registerConfig)
 * move init to have the same interface for all type of servers
 * move servers in DLL, the .exe would just host the one the user need to use
 *  -> separate config
 *  -> warning: GS with auth in same exe: delay connectToAuth ?
 * manage more field in TS_AG_CLIENT_LOGIN (play time)
 * Add API to do logging with static classes in Object (avoid crash using Log::get()->log(...))
 * Add log to LogServer (using LS_11N4S)
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

void onTerminate(void* instance) {
	ServersManager* serverManager = (ServersManager*) instance;

	if(serverManager)
		serverManager->stop();
}

int main(int argc, char **argv) {
	RappelzLibInit();
	GlobalConfig::init();

	DbConnectionPool dbConnectionPool;

	if(AuthServer::DB_Account::init(&dbConnectionPool, CONFIG_GET()->auth.client.desKey) == false)
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

	if(dbConnectionPool.checkConnection(CONFIG_GET()->auth.db.connectionString.get().c_str()) == false) {
		if(CONFIG_GET()->auth.db.ignoreInitCheck.get() == false)
			return 2;
	}

	CrashHandler::setDumpMode(CONFIG_GET()->admin.dumpMode);

	runServers(&trafficLogger);

	//Make valgrind happy
	AuthServer::DB_UpdateLastServerIdx::deinit();
	AuthServer::DB_Account::deinit();
	EventLoop::getInstance()->deleteObjects();

	return 0;
}

void runServers(Log *trafficLogger) {
	ServersManager serverManager;
	BanManager banManager;

	RappelzServer<AuthServer::ClientSession> authClientServer(&CONFIG_GET()->auth.client.listener.idleTimeout, trafficLogger);
	RappelzServer<AuthServer::GameServerSession> authGameServer(&CONFIG_GET()->auth.game.listener.idleTimeout, trafficLogger);
	RappelzServer<AuthServer::BillingInterface> billingTelnetServer(&CONFIG_GET()->auth.billing.listener.idleTimeout, trafficLogger);

	RappelzServer<UploadServer::ClientSession> uploadClientServer(&CONFIG_GET()->upload.client.listener.idleTimeout, trafficLogger);
	RappelzServer<UploadServer::IconServerSession> uploadIconServer(&CONFIG_GET()->upload.icons.listener.idleTimeout, trafficLogger);
	RappelzServer<UploadServer::GameServerSession> uploadGameServer(&CONFIG_GET()->upload.game.listener.idleTimeout, trafficLogger);

	RappelzServer<AdminServer::AdminInterface> adminTelnetServer(&CONFIG_GET()->admin.listener.idleTimeout);

	serverManager.addServer("auth.clients", &authClientServer,
							CONFIG_GET()->auth.client.listener.listenIp,
							CONFIG_GET()->auth.client.listener.port,
							CONFIG_GET()->auth.client.listener.autoStart,
							&banManager);
	serverManager.addServer("auth.gameserver", &authGameServer,
							CONFIG_GET()->auth.game.listener.listenIp,
							CONFIG_GET()->auth.game.listener.port,
							CONFIG_GET()->auth.game.listener.autoStart);
	serverManager.addServer("auth.billing", &billingTelnetServer,
							CONFIG_GET()->auth.billing.listener.listenIp,
							CONFIG_GET()->auth.billing.listener.port,
							CONFIG_GET()->auth.billing.listener.autoStart);

	serverManager.addServer("upload.clients", &uploadClientServer,
							CONFIG_GET()->upload.client.listener.listenIp,
							CONFIG_GET()->upload.client.listener.port,
							CONFIG_GET()->upload.client.listener.autoStart,
							&banManager);
	serverManager.addServer("upload.iconserver", &uploadIconServer,
							CONFIG_GET()->upload.icons.listener.listenIp,
							CONFIG_GET()->upload.icons.listener.port,
							CONFIG_GET()->upload.icons.listener.autoStart,
							&banManager);
	serverManager.addServer("upload.gameserver", &uploadGameServer,
							CONFIG_GET()->upload.game.listener.listenIp,
							CONFIG_GET()->upload.game.listener.port,
							CONFIG_GET()->upload.game.listener.autoStart);

	serverManager.addServer("admin.telnet", &adminTelnetServer,
							CONFIG_GET()->admin.listener.listenIp,
							CONFIG_GET()->admin.listener.port,
							CONFIG_GET()->admin.listener.autoStart);

	serverManager.start();


	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	CrashHandler::setTerminateCallback(nullptr, nullptr);
}
