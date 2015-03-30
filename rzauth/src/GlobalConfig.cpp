#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"
#include "rzauthGitVersion.h"
#include "DesPasswordCipher.h"

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
	CFG_CREATE("global.version", rzauthVersion);
	GlobalCoreConfig::get()->app.appName.setDefault("rzauth");
	GlobalCoreConfig::get()->app.configfile.setDefault("auth.opt");
	GlobalCoreConfig::get()->log.file.setDefault("auth.log");

#ifdef _WIN32
	GlobalConfig::get()->auth.db.driver.setDefault("SQL Server");
#else
	GlobalConfig::get()->auth.db.driver.setDefault("FreeTDS");
#endif

	srand((unsigned int)time(NULL));
}

void DbConfig::updateConnectionString(IListener* instance) {
	DbConfig* thisInstance = (DbConfig*)instance;

	std::string password;
	std::string cryptedPassword = thisInstance->cryptedPassword.get();
	if(cryptedPassword.size() > 0) {
		DesPasswordCipher("!_a^Rc*|#][Ych$~'(M _!d4aUo^%${T!~}h*&X%").decrypt(&cryptedPassword[0], (int)cryptedPassword.size());
		password = cryptedPassword;
	} else {
		password = thisInstance->password.get();
	}

	thisInstance->connectionString.setDefault(
				"driver=" + thisInstance->driver.get() +
				";Server=" + thisInstance->server.get() +
				"," + std::to_string((long long)thisInstance->port.get()) +
				";Database=" + thisInstance->name.get() +
				";UID=" + thisInstance->account.get() +
				";PWD=" + password +
				";");
}
