#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Console/ConsoleSession.h"
#include "Core/CrashHandler.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "GlobalConfig.h"
#include "LibRzuInit.h"
#include "NetSession/ServersManager.h"

#include "AuctionManager.h"

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
	AuctionManager auctionManager;
	ConsoleServer consoleServer(&serverManager);

	serverManager.addServer("auction.monitor", &auctionManager, nullptr);

	serverManager.start();

	CrashHandler::setTerminateCallback(&onTerminate, &serverManager);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	CrashHandler::setTerminateCallback(nullptr, nullptr);
}
