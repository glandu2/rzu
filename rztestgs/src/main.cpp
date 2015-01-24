#include "PlayerCountMonitor.h"
#include "uv.h"
#include <string.h>
#include "LibRzuInit.h"
#include "ConfigInfo.h"
#include "rzplayercountGitVersion.h"
#include "EventLoop.h"
#include "GlobalCoreConfig.h"

static void init();

int main(int argc, char *argv[])
{
	LibRzuInit();
	init();
	ConfigInfo::get()->init(argc, argv);
	ConfigInfo::get()->dump();

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
				   GlobalCoreConfig::get()->log.level,
				   GlobalCoreConfig::get()->log.consoleLevel,
				   GlobalCoreConfig::get()->log.dir,
				   GlobalCoreConfig::get()->log.file,
				   GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);


	PlayerCountMonitor playerCount(CFG_GET("ip")->getString(), CFG_GET("port")->getInt(), CFG_GET("req")->getString(), CFG_GET("interval")->getInt());
	playerCount.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

static void init() {
	CFG_CREATE("global.version", rzplayercountVersion);
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4500);
	CFG_CREATE("req", "TEST");
	CFG_CREATE("interval", 3500);
}
