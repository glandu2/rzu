#include "Core/EventLoop.h"
#include "GlobalConfig.h"
#include "LibRzuInit.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/CrashHandler.h"
#include "Database/DbConnectionPool.h"

#include "NetSession/ServersManager.h"
#include "NetSession/BanManager.h"
#include "NetSession/SessionServer.h"

#include "LogServer/ClientSession.h"

#include "AdminServer/AdminInterface.h"

void runServers(Log* trafficLogger);

void onTerminate(void* instance) {
	ServersManager* serverManager = (ServersManager*) instance;

	if(serverManager)
		serverManager->forceStop();
}

int main(int argc, char **argv) {
	LibRzuInit();
	GlobalConfig::init();
	BanManager::registerConfig();

	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
				   GlobalCoreConfig::get()->log.level,
				   GlobalCoreConfig::get()->log.consoleLevel,
				   GlobalCoreConfig::get()->log.dir,
				   GlobalCoreConfig::get()->log.file,
				   GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	Log trafficLogger(CONFIG_GET()->trafficDump.enable,
					  CONFIG_GET()->trafficDump.level,
					  CONFIG_GET()->trafficDump.consoleLevel,
					  CONFIG_GET()->trafficDump.dir,
					  CONFIG_GET()->trafficDump.file,
					  GlobalCoreConfig::get()->log.maxQueueSize);


	ConfigInfo::get()->dump();

	CrashHandler::setDumpMode(CONFIG_GET()->admin.dumpMode);

	runServers(&trafficLogger);

	//Make valgrind happy
	EventLoop::getInstance()->deleteObjects();

	return 0;
}

void runServers(Log *trafficLogger) {
	ServersManager serverManager;
	BanManager banManager;

	SessionServer<LogServer::ClientSession> logClientServer(
				CONFIG_GET()->log.client.listener.listenIp,
				CONFIG_GET()->log.client.listener.port,
				&CONFIG_GET()->log.client.listener.idleTimeout,
				trafficLogger,
				&banManager);

	SessionServer<AdminServer::AdminInterface> adminTelnetServer(
				CONFIG_GET()->admin.listener.listenIp,
				CONFIG_GET()->admin.listener.port,
				&CONFIG_GET()->admin.listener.idleTimeout);


	serverManager.addServer("log.clients", &logClientServer, CONFIG_GET()->log.client.listener.autoStart);
	serverManager.addServer("admin.telnet", &adminTelnetServer, CONFIG_GET()->admin.listener.autoStart);

	serverManager.start();

	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	CrashHandler::setTerminateCallback(nullptr, nullptr);
}
