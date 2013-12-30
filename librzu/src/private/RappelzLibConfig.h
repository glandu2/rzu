#ifndef RAPPELZLIBCONFIG_H
#define RAPPELZLIBCONFIG_H

#include "ConfigInfo.h"

struct RappelzLibConfig
{
	struct App {
		cval<std::string> &appName;

		App() : appName(CFG("core.appname", "Rappelz Auth Server")) {}
	} app;

	struct Log {
		cval<bool> &enable;
		cval<std::string> &dir, &file, &level;

		Log() :
			enable(CFG("core.log.enable", true)),
			dir(CFG("core.log.dir", "log")),
			file(CFG("core.log.file", CFG("core.appname", "auth").get() + ".log")),
			level(CFG("core.log.level", "trace")) {}
	} log;

	struct TrafficDump {
		cval<bool> &enable;
		cval<std::string> &dir, &file;

		TrafficDump() :
			enable(CFG("core.trafficdump.enable", false)),
			dir(CFG("core.trafficdump.dir", "traffic_log")),
			file(CFG("core.trafficdump.file", "auth_traffic.log")) {}
	} trafficDump;

	static RappelzLibConfig* get();
};

#define CONFIG_GET() RappelzLibConfig::get()

#endif // RAPPELZLIBCONFIG_H
