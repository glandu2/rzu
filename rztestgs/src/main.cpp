#include "PlayerCountMonitor.h"
#include "uv.h"
#include <string.h>
#include "RappelzLibInit.h"
#include "ConfigInfo.h"
#include "PlayerCountGitVersion.h"
#include "EventLoop.h"
#include "RappelzLibConfig.h"

static void init();

int main(int argc, char *argv[])
{
	RappelzLibInit();
	init();
	ConfigInfo::get()->init(argc, argv);
	ConfigInfo::get()->dump();

	Log mainLogger(RappelzLibConfig::get()->log.enable,
				   RappelzLibConfig::get()->log.level,
				   RappelzLibConfig::get()->log.consoleLevel,
				   RappelzLibConfig::get()->log.dir,
				   RappelzLibConfig::get()->log.file,
				   RappelzLibConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);


	PlayerCountMonitor playerCount(CFG_GET("ip")->getString(), CFG_GET("port")->getInt(), CFG_GET("req")->getString(), CFG_GET("interval")->getInt());
	playerCount.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

static void init() {
	CFG_CREATE("global.version", PlayerCountVersion);
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4500);
	CFG_CREATE("req", "TEST");
	CFG_CREATE("interval", 3500);
}
