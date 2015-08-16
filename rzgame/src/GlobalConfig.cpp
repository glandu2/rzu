#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"
#include "rzgameGitVersion.h"
#include "Utils.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG_CREATE("global.version", rzgameVersion);
	GlobalCoreConfig::get()->app.appName.setDefault("rzgame");
	GlobalCoreConfig::get()->app.configfile.setDefault("game.opt");
	GlobalCoreConfig::get()->log.file.setDefault("game.log");

#ifdef _WIN32
	GlobalConfig::get()->game.db.driver.setDefault("SQL Server");
#else
	GlobalConfig::get()->game.db.driver.setDefault("FreeTDS");
#endif
}

void DbConfig::updateConnectionString(IListener* instance) {
	DbConfig* thisInstance = (DbConfig*)instance;

	thisInstance->connectionString.setDefault(
				"driver=" + thisInstance->driver.get() +
				";Server=" + thisInstance->server.get() +
				"," + Utils::convertToString(thisInstance->port.get()) +
				";Database=" + thisInstance->name.get() +
				";UID=" + thisInstance->account.get() +
				";PWD=" + thisInstance->password.get() +
				";");
}
