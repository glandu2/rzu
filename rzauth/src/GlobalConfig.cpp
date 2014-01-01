#include "GlobalConfig.h"
#include "RappelzLibConfig.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	RappelzLibConfig::get()->app.appName.setDefault("RappelzAuthEmu");
	RappelzLibConfig::get()->app.configfile.setDefault("auth.opt");
	RappelzLibConfig::get()->log.file.setDefault("auth.log");
	RappelzLibConfig::get()->trafficDump.file.setDefault("auth_traffic.log");
}
