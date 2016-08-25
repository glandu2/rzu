#include "GlobalConfig.h"
#include "Config/GlobalCoreConfig.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	GlobalCoreConfig::get()->app.appName.setDefault("rzauctionmonitor");
	GlobalCoreConfig::get()->app.configfile.setDefault("auctionmonitor.opt");
	GlobalCoreConfig::get()->log.file.setDefault("auctionmonitor.log");
}
