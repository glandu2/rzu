#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	GlobalCoreConfig::get()->log.level.setDefault("fatal");
	GlobalCoreConfig::get()->log.consoleLevel.setDefault("info");
}
