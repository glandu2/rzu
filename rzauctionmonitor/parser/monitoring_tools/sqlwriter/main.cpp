#include "AuctionParser.h"
#include "GlobalConfig.h"
#include "LibRzuInit.h"
#include "Core/EventLoop.h"
#include "NetSession/ServersManager.h"
#include "Core/CrashHandler.h"
#include "SqlWriter.h"
#include "Database/DbConnectionPool.h"
#include "Database/DbBindingLoader.h"
#include "AuctionSQLWriter.h"

static void onTerminate(void* instance) {
	ServersManager* serverManager = (ServersManager*) instance;

	if(serverManager) {
		serverManager->forceStop();
	}
}

int main(int argc, char* argv[]) {
	LibRzuInit();
	GlobalConfig::init();

	DbConnectionPool dbConnectionPool;
	DbBindingLoader::get()->initAll(&dbConnectionPool);

	ConfigInfo::get()->init(argc, argv);


	Log mainLogger(GlobalCoreConfig::get()->log.enable,
	               GlobalCoreConfig::get()->log.level,
	               GlobalCoreConfig::get()->log.consoleLevel,
	               GlobalCoreConfig::get()->log.dir,
	               GlobalCoreConfig::get()->log.file,
	               GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	ConfigInfo::get()->dump();

	ServersManager serverManager;
	SqlWriter sqlWriter;
	AuctionParser auctionParser(&sqlWriter,
	                            CONFIG_GET()->input.auctionsPath,
	                            CONFIG_GET()->input.changeWaitSeconds,
	                            CONFIG_GET()->states.statesPath,
	                            CONFIG_GET()->states.auctionStateFile,
	                            CONFIG_GET()->states.aggregationStateFile);

	serverManager.addServer("auction.monitor", &auctionParser, nullptr);

	//DB_Item::createTable(&dbConnectionPool);
	serverManager.start();

	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
	CrashHandler::setTerminateCallback(nullptr, nullptr);

	return 0;
}
