#pragma once

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"

struct GlobalConfig {
	struct WebserverConfig {
		cval<std::string>& ip;
		cval<int>& port;
		cval<std::string>& hostname;
		cval<std::string>& url;
		cval<std::string>& pwd;

		WebserverConfig()
		    : ip(CFG_CREATE("webserver.ip", "127.0.0.1")),
		      port(CFG_CREATE("webserver.port", 8080)),
		      hostname(CFG_CREATE("webserver.hostname", "127.0.0.1")),
		      url(CFG_CREATE("webserver.url", "/api/upload_auction")),
		      pwd(CFG_CREATE("webserver.pwd", "")) {}
	} webserver;

	struct DbConfig {
		cval<std::string>& connectionString;

		DbConfig()
		    : connectionString(
		          CFG_CREATE("db.connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;")) {}
	} db;

	struct InputConfig {
		cval<std::string>& auctionsPath;
		cval<int>& changeWaitSeconds;
		cval<bool>& autoStart;

		InputConfig()
		    : auctionsPath(CFG_CREATE("input.auctionpath", "auctions")),
		      changeWaitSeconds(CFG_CREATE("input.waitchangeseconds", 30)),
		      autoStart(CFG_CREATE("input.autostart", true)) {
			Utils::autoSetAbsoluteDir(auctionsPath);
		}
	} input;

	struct TrafficDump {
		cval<bool>& enable;
		cval<std::string>&dir, &file, &level, &consoleLevel;

		TrafficDump()
		    : enable(CFG_CREATE("trafficdump.enable", false)),
		      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		      file(CFG_CREATE("trafficdump.file", "auctionaggregator.log")),
		      level(CFG_CREATE("trafficdump.level", "debug")),
		      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal")) {
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()
