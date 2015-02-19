#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"
#include "rzgamereconnectGitVersion.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG_CREATE("global.version", rzgamereconnectVersion);
	GlobalCoreConfig::get()->app.appName.setDefault("rzgamereconnect");
	GlobalCoreConfig::get()->app.configfile.setDefault("rzgamereconnect.opt");
	GlobalCoreConfig::get()->log.file.setDefault("rzgamereconnect.log");

	srand((unsigned int)time(NULL));
}
