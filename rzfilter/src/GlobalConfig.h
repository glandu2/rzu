#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"
#include "Utils.h"

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
			ip(CFG_CREATE("server.ip", "255.255.255.1")), //user must change this
			port(CFG_CREATE("server.port", 4500)) {}
	} server;

	struct GameConfig {
		ListenerConfig listener;
		cval<bool> &authMode; //if enabled, filter server list IPs
		cval<std::string> &gameFilterIp;
		cval<int> &gameFilterPort; //if -1, no change

		GameConfig() :
			listener("client.listen", "127.0.0.1", 4500, true, 0),
			authMode(CFG_CREATE("client.authmode", false)),
			gameFilterIp(CFG_CREATE("client.gamefilter.ip", "127.0.0.1")),
			gameFilterPort(CFG_CREATE("client.gamefilter.port", -1))
		{}
	} client;

	struct TrafficDump {
		cval<bool> &enable;
		cval<std::string> &dir, &file, &level, &consoleLevel;

		TrafficDump() :
			enable(CFG_CREATE("trafficdump.enable", true)),
			dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
			file(CFG_CREATE("trafficdump.file", "rzfilter.log")),
			level(CFG_CREATE("trafficdump.level", "debug")),
			consoleLevel(CFG_CREATE("trafficdump.consolelevel", "info"))
		{
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
