#pragma once

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"

struct GlobalConfig {
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

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

