#pragma once

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/Utils.h"

struct GlobalConfig {
	struct AuthConfig {
		cval<std::string>& ip;
		cval<int>& port;
		cval<int>& reconnectDelay;

		AuthConfig()
		    : ip(CFG_CREATE("auth.ip", "127.0.0.1")),
		      port(CFG_CREATE("auth.port", 4502)),
		      reconnectDelay(CFG_CREATE("auth.reconnectdelay", 5000)) {}
	} auth;

	struct GameConfig {
		ListenerConfig listener;

		GameConfig() : listener("game", "127.0.0.1", 4802, true, 0) {}
	} game;

	struct TrafficDump {
		cval<bool>& enable;
		cval<std::string>&dir, &file, &level, &consoleLevel;

		TrafficDump()
		    : enable(CFG_CREATE("trafficdump.enable", false)),
		      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		      file(CFG_CREATE("trafficdump.file", "rzgamereconnect.log")),
		      level(CFG_CREATE("trafficdump.level", "debug")),
		      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal")) {
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

