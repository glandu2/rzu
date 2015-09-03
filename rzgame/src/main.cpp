#include "Core/EventLoop.h"
#include "GlobalConfig.h"
#include "LibRzuInit.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/CrashHandler.h"
#include "Database/DbConnectionPool.h"

#include "NetSession/ServersManager.h"
#include "NetSession/BanManager.h"
#include "NetSession/SessionServer.h"

#include "GameServer/AuthServerSession.h"
#include "GameServer/ClientSession.h"
#include "GameServer/Database/CharacterList.h"

#include "Console/ConsoleSession.h"

void runServers(Log* trafficLogger);

void onTerminate(void* instance) {
	ServersManager* serverManager = (ServersManager*) instance;

	if(serverManager)
		serverManager->forceStop();

	uv_loop_close(EventLoop::getLoop());
}

int main(int argc, char **argv) {
	LibRzuInit();
	GlobalConfig::init();
	BanManager::registerConfig();

	GameServer::AuthServerSession::init();

	DbConnectionPool dbConnectionPool;

	if(DbQueryJob<Database::CharacterList>::init(&dbConnectionPool) == false)
		return 1;

	ConfigInfo::get()->init(argc, argv);

	{
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
		Log::setDefaultPacketLogger(&trafficLogger);

		ConfigInfo::get()->dump();

		dbConnectionPool.checkConnection(CONFIG_GET()->game.db.connectionString.get().c_str());

		CrashHandler::setDumpMode(CONFIG_GET()->admin.dumpMode);

		runServers(&trafficLogger);

		//Make valgrind happy
		GameServer::AuthServerSession::deinit();
		EventLoop::getInstance()->deleteObjects();
	}

	return 0;
}

void runServers(Log *trafficLogger) {
	ServersManager serverManager;
	BanManager banManager;

	SessionServer<ConsoleSession> adminTelnetServer(
				CONFIG_GET()->admin.telnet.listenIp,
				CONFIG_GET()->admin.telnet.port);
	SessionServer<GameServer::ClientSession> clientsServer(
				CONFIG_GET()->game.clients.listenIp,
				CONFIG_GET()->game.clients.port,
				nullptr,
				trafficLogger);
	GameServer::AuthServerSession authConnection;

	banManager.loadFile();

	serverManager.addServer("admin.telnet", &adminTelnetServer,
							CONFIG_GET()->admin.telnet.autoStart, true);

	serverManager.addServer("game.clients", &clientsServer,
							CONFIG_GET()->game.clients.autoStart);

	serverManager.addServer("game.auth", &authConnection,
							authConnection.getAutoStartConfig());

	serverManager.start();

	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	CrashHandler::setTerminateCallback(nullptr, nullptr);
}
