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

#ifdef _WIN32
	GlobalConfig::get()->log.db.driver.setDefault("SQL Server");
#else
	GlobalConfig::get()->log.db.driver.setDefault("FreeTDS");
#endif
}

void DbConfig::updateConnectionString(IListener* instance) {
	DbConfig* thisInstance = (DbConfig*)instance;

	std::string cryptedConnectionStringHex = thisInstance->cryptedConnectionString.get();
	if(cryptedConnectionStringHex.size() > 0) {
		thisInstance->connectionString.setDefault(DbPasswordCipher::decrypt(Utils::convertHexToData(cryptedConnectionStringHex)));
	} else {
		std::string password;
		std::string cryptedPasswordHex = thisInstance->cryptedPassword.get();
		if(cryptedPasswordHex.size() > 0) {
			password = DbPasswordCipher::decrypt(Utils::convertHexToData(cryptedPasswordHex));
		} else {
			password = thisInstance->password.get();
		}

		thisInstance->connectionString.setDefault(
					"driver=" + thisInstance->driver.get() +
					";Server=" + thisInstance->server.get() +
					"," + Utils::convertToString(thisInstance->port.get()) +
					";Database=" + thisInstance->name.get() +
					";UID=" + thisInstance->account.get() +
					";PWD=" + password +
					";");
	}

}
