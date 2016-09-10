#include "GlobalConfig.h"
#include "Config/GlobalCoreConfig.h"
#include "rzfilterGitVersion.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG_CREATE("global.version", rzfilterVersion);
	GlobalCoreConfig::get()->admin.listener.port.setDefault(4803);
}
