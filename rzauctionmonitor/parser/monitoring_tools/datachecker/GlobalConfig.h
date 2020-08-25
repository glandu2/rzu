#pragma once

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"

struct GlobalConfig {
	struct InputConfig {
		cval<std::string>& auctionsPath;
		cval<int>& changeWaitSeconds;
		cval<std::string>& firstFileToParse;

		InputConfig()
		    : auctionsPath(CFG_CREATE("input.auctionpath", "auctions")),
		      changeWaitSeconds(CFG_CREATE("input.waitchangeseconds", 30)),
		      firstFileToParse(CFG_CREATE("input.firstfiletoparse", "")) {
			Utils::autoSetAbsoluteDir(auctionsPath);
		}
	} input;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()
