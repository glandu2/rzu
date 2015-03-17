#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"
#include "rzfilterGitVersion.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG_CREATE("global.version", rzfilterVersion);
	GlobalCoreConfig::get()->app.appName.setDefault("rzfilter");
	GlobalCoreConfig::get()->app.configfile.setDefault("rzfilter.opt");
	GlobalCoreConfig::get()->log.file.setDefault("rzfilter.log");
}
