#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"

GlobalTestConfig* GlobalTestConfig::get() {
	static GlobalTestConfig config;
	return &config;
}

void GlobalTestConfig::init() {
	GlobalTestConfig::get();
	GlobalCoreConfig::get()->app.appName.setDefault("rztest");
	GlobalCoreConfig::get()->app.configfile.setDefault("test.opt");
	GlobalCoreConfig::get()->log.file.setDefault("test.log");
	GlobalCoreConfig::get()->log.level.setDefault("fatal");
	GlobalCoreConfig::get()->log.consoleLevel.setDefault("info");

	srand((unsigned int)time(NULL));
}
