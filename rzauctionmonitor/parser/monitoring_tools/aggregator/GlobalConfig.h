#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

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

	struct InputConfig {
		cval<std::string>& auctionsPath;
		cval<int>& changeWaitSeconds;

		InputConfig()
		    : auctionsPath(CFG_CREATE("input.auctionpath", "auctions")),
		      changeWaitSeconds(CFG_CREATE("input.waitchangeseconds", 30)) {
			Utils::autoSetAbsoluteDir(auctionsPath);
		}
	} input;

	struct StatesConfig {
		cval<std::string>& statesPath;
		cval<std::string>& auctionStateFile;
		cval<std::string>& aggregationStateFile;

		StatesConfig()
		    : statesPath(CFG_CREATE("state.path", "parser_states")),
		      auctionStateFile(CFG_CREATE("state.auctionfile", "auction_state.bin")),
		      aggregationStateFile(CFG_CREATE("state.aggregationfile", "aggregation_state.bin")) {
			Utils::autoSetAbsoluteDir(statesPath);
		}
	} states;

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

#endif  // GLOBALCONFIG_H
