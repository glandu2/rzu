#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"
#include "Core/Utils.h"

struct ListenerConfig {
	cval<std::string> &listenIp;
	cval<int> &port, &idleTimeout;
	cval<bool> &autoStart;

	ListenerConfig(const std::string& prefix, const char* defaultIp, int defaultPort, bool autoStart = true, int idleTimeout = 0) :
		listenIp(CFG_CREATE(prefix + ".ip", defaultIp)),
		port(CFG_CREATE(prefix + ".port", defaultPort)),
		idleTimeout(CFG_CREATE(prefix + ".idletimeout", idleTimeout)),
		autoStart(CFG_CREATE(prefix + ".autostart", autoStart))
	{}
};

struct GlobalConfig {

	struct LogConfig {
		cval<std::string> &logDir;

		struct ClientConfig {
			ListenerConfig listener;

			ClientConfig() :
				listener("log.clients", "127.0.0.1", 4516, true, 0) {}
		} client;

		LogConfig() :
			logDir(CFG_CREATE("log.dir", "log"))
		{
			Utils::autoSetAbsoluteDir(logDir);
		}
	} log;

	struct AdminConfig {
		ListenerConfig listener;
		cval<int> &dumpMode;

		AdminConfig() :
			listener("admin.telnet", "127.0.0.1", 4517, true, 0),
			dumpMode(CFG_CREATE("admin.dump_mode", 0)) //1: no dump, anything else: create dump on crash
		{}
	} admin;

	struct TrafficDump {
		cval<bool> &enable;
		cval<std::string> &dir, &file, &level, &consoleLevel;

		TrafficDump() :
			enable(CFG_CREATE("trafficdump.enable", false)),
			dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
			file(CFG_CREATE("trafficdump.file", "rzlog.log")),
			level(CFG_CREATE("trafficdump.level", "debug")),
			consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal"))
		{
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
