#include "GlobalConfig.h"
#include "Config/GlobalCoreConfig.h"
#include "rzclientreconnectGitVersion.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG_CREATE("global.version", rzclientreconnectVersion);
	GlobalCoreConfig::get()->admin.listener.port.setDefault(4801);
}

void GlobalConfig::GameConfig::updateDefaultExternalIp(IListener* instance) {
	GlobalConfig::GameConfig* thisInstance = (GlobalConfig::GameConfig*) instance;

	thisInstance->gameExternalIp.setDefault(thisInstance->listener.listenIp.get());
}
