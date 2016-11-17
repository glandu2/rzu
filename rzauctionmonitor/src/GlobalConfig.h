#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"

struct GlobalConfig {
	struct ClientConfig {
		cval<bool> &useRsa;
		cval<std::string> &ip, &accountFile, &auctionListDir, &auctionListFile;
		cval<int> &port, &gsindex, &autoRecoDelay, &recoDelay, &recoTimeout, &auctionSearchDelay, &auctionSearchTimeout;
		cval<bool> &doFullAuctionDump, &doStateAuctionDump;
		cval<std::string> &stateFile;
		cval<bool> &autoStart;

		ClientConfig() :
			useRsa(CFG_CREATE("client.use_rsa", true)),
			ip(CFG_CREATE("client.ip", "127.0.0.1")),
			accountFile(CFG_CREATE("client.account_file", "accounts.txt")),
			auctionListDir(CFG_CREATE("client.auctions_dir", "auctions")),
			auctionListFile(CFG_CREATE("client.auctions_file", "auctions.bin.gz")),
			port(CFG_CREATE("client.port", 4500)),
			gsindex(CFG_CREATE("client.gs_index", 1)),
			autoRecoDelay(CFG_CREATE("client.auto_reco_delay", 0)),
			recoDelay(CFG_CREATE("client.reco_delay", 5000)),
			recoTimeout(CFG_CREATE("client.reco_timeout", 10000)),
			auctionSearchDelay(CFG_CREATE("client.auction_search_delay", 3000)),
			auctionSearchTimeout(CFG_CREATE("client.auction_search_timeout", 5000)),
		    doFullAuctionDump(CFG_CREATE("client.do_full_auction_dump", false)),
		    doStateAuctionDump(CFG_CREATE("client.do_state_auction_dump", false)),
		    stateFile(CFG_CREATE("client.initial_state_file", "")),
		    autoStart(CFG_CREATE("client.autostart", true))
		{
			Utils::autoSetAbsoluteDir(auctionListDir);
		}
	} client;

	struct TrafficDump {
		cval<bool> &enable;
		cval<std::string> &dir, &file, &level, &consoleLevel;

		TrafficDump() :
			enable(CFG_CREATE("trafficdump.enable", false)),
			dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
			file(CFG_CREATE("trafficdump.file", "auctionmonitor.log")),
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
