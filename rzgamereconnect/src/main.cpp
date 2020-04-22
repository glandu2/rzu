#include "Config/GlobalCoreConfig.h"
#include "Core/CrashHandler.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "Database/DbConnectionPool.h"
#include "GlobalConfig.h"
#include "LibRzuInit.h"

#include "NetSession/BanManager.h"
#include "NetSession/ServersManager.h"
#include "NetSession/SessionServer.h"

#include "AuthServer/AuthSession.h"
#include "AuthServer/GameServerSession.h"

#include "Console/ConsoleSession.h"

#include <stdlib.h>

void runServers(Log* trafficLogger);

void onTerminate(void* instance) {
	ServersManager* serverManager = (ServersManager*) instance;

	if(serverManager)
		serverManager->forceStop();
}

int main(int argc, char** argv) {
	LibRzuScopedUse useLibRzu;
	GlobalConfig::init();
	AuthServer::AuthSession::init();

	srand(time(nullptr) ^ (time_t) argv);

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

	runServers(&trafficLogger);

	// Make valgrind happy
	EventLoop::getInstance()->deleteObjects();

	return 0;
}

void runServers(Log* trafficLogger) {
	ServersManager serverManager;

	SessionServer<AuthServer::GameServerSession> authGameServer(CONFIG_GET()->game.listener.listenIp,
	                                                            CONFIG_GET()->game.listener.port,
	                                                            &CONFIG_GET()->game.listener.idleTimeout,
	                                                            trafficLogger);

	serverManager.addServer("auth.gameserver", &authGameServer, &CONFIG_GET()->game.listener.autoStart);
	ConsoleServer consoleServer(&serverManager);

	serverManager.start();

	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	CrashHandler::setTerminateCallback(nullptr, nullptr);
}
