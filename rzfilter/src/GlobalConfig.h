#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"
#include "Core/Utils.h"
#include "Config/GlobalCoreConfig.h"
#include "Packet/PacketEpics.h"

struct GlobalConfig {
	struct AuthConfig {
		cval<std::string> &ip;
		cval<int> &port;
		cval<int> &epic;

		AuthConfig() :
		    ip(CFG_CREATE("server.ip", "remote.auth.server.ip")), //user must change this
		    port(CFG_CREATE("server.port", 4500)),
		    epic(CFG_CREATE("server.epic", EPIC_LATEST))
		{}
	} server;

	struct GameConfig {
		ListenerConfig listener;
		cval<bool> &authMode; //if enabled, filter server list IPs
		cval<std::string> &gameFilterIp;
		cval<int> &gameFilterPort; //if -1, no change
		cval<int> &epic;

		GameConfig() :
		    listener("client.listen", "127.0.0.1", 4500, true, 0),
		    authMode(CFG_CREATE("client.authmode", false)),
		    gameFilterIp(CFG_CREATE("client.gamefilter.ip", "127.0.0.1")),
		    gameFilterPort(CFG_CREATE("client.gamefilter.port", -1)),
		    epic(CFG_CREATE("client.epic", EPIC_LATEST))
		{}
	} client;

	struct FilterConfig {
		cval<std::string> &filterModuleName;

		FilterConfig() :
		    filterModuleName(CFG_CREATE("filter.modulename", "rzfilter_module"))
		{}
	} filter;

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
