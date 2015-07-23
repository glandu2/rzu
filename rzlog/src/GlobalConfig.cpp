#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"
#include "rzlogGitVersion.h"
#include "DbPasswordCipher.h"
#include "Utils.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG_CREATE("global.version", rzlogVersion);
	GlobalCoreConfig::get()->app.appName.setDefault("rzlog");
	GlobalCoreConfig::get()->app.configfile.setDefault("rzlog.opt");
	GlobalCoreConfig::get()->log.file.setDefault("rzlog.log");
}
