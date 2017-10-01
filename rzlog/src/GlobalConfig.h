#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/Utils.h"

struct GlobalConfig {
	struct LogConfig {
		cval<std::string>& logDir;

		struct ClientConfig {
			ListenerConfig listener;

			ClientConfig() : listener("log.clients", "127.0.0.1", 4516, true, 0) {}
		} client;

		LogConfig() : logDir(CFG_CREATE("log.dir", "log")) { Utils::autoSetAbsoluteDir(logDir); }
	} log;

	struct TrafficDump {
		cval<bool>& enable;
		cval<std::string>&dir, &file, &level, &consoleLevel;

		TrafficDump()
		    : enable(CFG_CREATE("trafficdump.enable", false)),
		      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		      file(CFG_CREATE("trafficdump.file", "rzlog.log")),
		      level(CFG_CREATE("trafficdump.level", "debug")),
		      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal")) {
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif  // GLOBALCONFIG_H
