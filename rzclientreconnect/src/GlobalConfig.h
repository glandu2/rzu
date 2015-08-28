#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"
#include "Core/Utils.h"

struct ListenerConfig {
	cval<std::string> &ip;
	cval<int> &port, &idleTimeout;
	cval<bool> &autoStart;

	ListenerConfig(const std::string& prefix, const char* defaultIp, int defaultPort, bool autoStart = true, int idleTimeout = 0) :
		ip(CFG_CREATE(prefix + ".ip", defaultIp)),
		port(CFG_CREATE(prefix + ".port", defaultPort)),
		idleTimeout(CFG_CREATE(prefix + ".idletimeout", idleTimeout)),
		autoStart(CFG_CREATE(prefix + ".autostart", autoStart))
	{}
};

struct GlobalConfig {
	struct AuthConfig {
		cval<std::string> &ip;
		cval<int> &port;

		AuthConfig() :
			ip(CFG_CREATE("auth.ip", "127.0.0.1")),
			port(CFG_CREATE("auth.port", 4502)) {}
	} auth;

	struct GameConfig {
		ListenerConfig listener;

		GameConfig() :
			listener("game", "127.0.0.1", 4802, true, 0) {}
	} game;

	struct AdminConfig {
		ListenerConfig listener;
		cval<int> &dumpMode;

		AdminConfig() :
			listener("admin.telnet", "127.0.0.1", 4801, true, 0),
			dumpMode(CFG_CREATE("admin.dump_mode", 0)) //1: no dump, anything else: create dump on crash
		{}
	} admin;

	struct TrafficDump {
		cval<bool> &enable;
		cval<std::string> &dir, &file, &level, &consoleLevel;

		TrafficDump() :
			enable(CFG_CREATE("trafficdump.enable", false)),
			dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
			file(CFG_CREATE("trafficdump.file", "rzgamereconnect.log")),
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
