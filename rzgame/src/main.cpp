#include "Core/EventLoop.h"
#include "GlobalConfig.h"
#include "LibRzuInit.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/CrashHandler.h"
#include "Database/DbConnectionPool.h"

#include "NetSession/ServersManager.h"
#include "NetSession/BanManager.h"
#include "NetSession/SessionServer.h"

#include "GameServer/ReferenceData/ReferenceDataMgr.h"
#include "GameServer/AuthServerSession.h"
#include "GameServer/ClientSession.h"

#include "Console/ConsoleSession.h"

static void runServers(Log* trafficLogger);
static void onReferenceDataLoaded(void* data);

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

	DbBindingLoader::get()->initAll(&dbConnectionPool);

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

		dbConnectionPool.checkConnection(CONFIG_GET()->game.arcadia.connectionString.get().c_str());
		dbConnectionPool.checkConnection(CONFIG_GET()->game.telecaster.connectionString.get().c_str());

		runServers(&trafficLogger);

		//Make valgrind happy
		GameServer::AuthServerSession::deinit();
		EventLoop::getInstance()->deleteObjects();
	}

	return 0;
}

static void runServers(Log *trafficLogger) {
	ServersManager serverManager;
	BanManager banManager;

	SessionServer<GameServer::ClientSession> clientsServer(
				CONFIG_GET()->game.clients.listenIp,
				CONFIG_GET()->game.clients.port,
				nullptr,
				trafficLogger);
	GameServer::AuthServerSession authConnection;

	banManager.loadFile();

	ConsoleSession::start(&serverManager);

	serverManager.addServer("game.clients", &clientsServer,
							CONFIG_GET()->game.clients.autoStart);

	serverManager.addServer("game.auth", &authConnection,
							authConnection.getAutoStartConfig());

	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);

	GameServer::ReferenceDataMgr::load(&onReferenceDataLoaded, &serverManager);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	CrashHandler::setTerminateCallback(nullptr, nullptr);
}

static void onReferenceDataLoaded(void* data) {
	ServersManager* serverManager = (ServersManager*) data;
	serverManager->start();

	int epic = CONFIG_GET()->game.clients.epic;
	Object::logStatic(Object::LL_Info, "main", "Target epic: %x.%x.%x\n", epic >> 16, (epic >> 8) & 0xFF, epic & 0xFF);
}
