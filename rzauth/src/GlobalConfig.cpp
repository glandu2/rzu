#include "GlobalConfig.h"
#include "RappelzLibConfig.h"
#include "RappelzServerAuthGitVersion.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG("global.version", RappelzServerAuthVersion);
	RappelzLibConfig::get()->app.appName.setDefault("RappelzAuthEmu");
	RappelzLibConfig::get()->app.configfile.setDefault("auth.opt");
	RappelzLibConfig::get()->log.file.setDefault("auth.log");
	RappelzLibConfig::get()->trafficDump.file.setDefault("auth_traffic.log");

#ifdef _WIN32
	GlobalConfig::get()->auth.dbAccount.driver.setDefault("SQL Server");
//	GlobalConfig::get()->ban.dbBan.driver.setDefault("SQL Server");
#else
	GlobalConfig::get()->auth.dbAccount.driver.setDefault("FreeTDS");
//	GlobalConfig::get()->ban.dbBan.driver.setDefault("FreeTDS");
#endif

	srand((unsigned int)time(NULL));
}

void DbConfig::updateConnectionString(IListener* instance) {
	DbConfig* thisInstance = (DbConfig*)instance;

	thisInstance->connectionString.setDefault(
				"driver=" + thisInstance->driver.get() +
				";Server=" + thisInstance->server.get() +
				";Database=" + thisInstance->name.get() +
				";UID=" + thisInstance->account.get() +
				";PWD=" + thisInstance->password.get() +
				";Port=" + std::to_string((long long)thisInstance->port.get()) +
				";");
}
