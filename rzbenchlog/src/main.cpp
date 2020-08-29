#include <string>

#include "BenchmarkLogSession.h"
#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "LibRzuInit.h"

class IListener;

static void init() {
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4516);
	CFG_CREATE("count", 40);
	CFG_CREATE("targetcount", 3000);
	CFG_CREATE("delay", 1);
}

int main(int argc, char* argv[]) {
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

	BenchmarkConfig config;
	config.packetSent = 0;
	config.packetPerSalve = CFG_GET("count")->getInt();
	config.packetTargetCount = CFG_GET("targetcount")->getInt();
	config.delay = CFG_GET("delay")->getInt();

	std::string ip = CFG_GET("ip")->getString();
	int port = CFG_GET("port")->getInt();

	BenchmarkLogSession session(&config);

	session.connect(ip.c_str(), port);

	mainLogger.log(Object::LL_Info, "main", 4, "Starting benchmark\n");

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}
