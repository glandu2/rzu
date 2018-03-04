#include "Aggregator.h"
#include "AuctionParser.h"
#include "Core/CrashHandler.h"
#include "Core/EventLoop.h"
#include "GlobalConfig.h"
#include "LibRzuInit.h"
#include "NetSession/ServersManager.h"

static void onTerminate(void* instance) {
	ServersManager* serverManager = (ServersManager*) instance;

	if(serverManager) {
		serverManager->forceStop();
	}
}

int main(int argc, char* argv[]) {
	LibRzuScopedUse useLibRzu;
	GlobalConfig::init();

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
	Log::setDefaultPacketLogger(&trafficLogger);

	ConfigInfo::get()->dump();

	ServersManager serverManager;
	Aggregator aggregator;
	AuctionParser auctionParser(&aggregator,
	                            CONFIG_GET()->input.auctionsPath,
	                            CONFIG_GET()->input.changeWaitSeconds,
	                            CONFIG_GET()->states.statesPath,
	                            CONFIG_GET()->states.auctionStateFile,
	                            CONFIG_GET()->states.aggregationStateFile);

	serverManager.addServer("auction.monitor", &auctionParser, nullptr);

	serverManager.start();

	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
	CrashHandler::setTerminateCallback(nullptr, nullptr);

	return 0;
}
