#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "LibRzuInit.h"
#include "TestGs.h"
#include "rztestgsGitVersion.h"
#include "uv.h"
#include <string.h>

static void init();

int main(int argc, char* argv[]) {
	LibRzuScopedUse useLibRzu;
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

	TestGs testGs(CFG_GET("ip")->getString(), CFG_GET("port")->getInt(), CFG_GET("req")->getString());
	testGs.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

static void init() {
	CFG_CREATE("global.version", rztestgsVersion);
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4500);
	CFG_CREATE("req", "TEST");
	CFG_CREATE("interval", 3500);
}
