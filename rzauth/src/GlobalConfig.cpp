#include "GlobalConfig.h"
#include "RappelzLibConfig.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	RappelzLibConfig::get()->app.appName.setDefault("RappelzAuthEmu");
	RappelzLibConfig::get()->app.configfile.setDefault("auth.opt");
	RappelzLibConfig::get()->log.file.setDefault("auth.log");
	RappelzLibConfig::get()->trafficDump.file.setDefault("auth_traffic.log");

#ifdef _WIN32
	GlobalConfig::get()->auth.dbAccount.driver.setDefault("SQL Server");
#else
	GlobalConfig::get()->auth.dbAccount.driver.setDefault("FreeTDS");
#endif
}

void GlobalConfig::AuthConfig::DbAccount::updateConnectionString(ICallbackGuard* instance) {
	DbAccount* thisInstance = (DbAccount*)instance;

	thisInstance->connectionString.setDefault(
				"driver=" + thisInstance->driver.get() +
				";Server=" + thisInstance->server.get() +
				";Database=" + thisInstance->name.get() +
				";UID=" + thisInstance->account.get() +
				";PWD=" + thisInstance->password.get() +
				";Port=" + std::to_string((long long)thisInstance->port.get()) +
				";");
}
