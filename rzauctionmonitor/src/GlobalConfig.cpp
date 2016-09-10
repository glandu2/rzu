#include "GlobalConfig.h"
#include "Config/GlobalCoreConfig.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
}
