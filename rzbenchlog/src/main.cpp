#include "uv.h"
#include "BenchmarkLogSession.h"
#include <string.h>
#include "Core/EventLoop.h"
#include "LibRzuInit.h"
#include <stdio.h>
#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "TimingFunctions.h"
#include "Core/PrintfFormats.h"
#include "Stream/Socket.h"
#include "Core/Utils.h"

void onSocketStateChange(IListener* instance, Stream *socket, Stream::State oldState, Stream::State newState, bool causedByRemote);

static void init() {
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4516);
	CFG_CREATE("count", 40);
	CFG_CREATE("targetcount", 3000);
	CFG_CREATE("delay", 1);
	GlobalCoreConfig::get()->app.appName.setDefault("rzbenchlog");
	GlobalCoreConfig::get()->app.configfile.setDefault("rzbenchlog.opt");
	GlobalCoreConfig::get()->log.file.setDefault("rzbenchlog.log");
}

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
