#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"
#include "rzauthGitVersion.h"
#include "DesPasswordCipher.h"
#include "ZlibCipher.h"

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
	std::string cryptedPasswordHex = thisInstance->cryptedPassword.get();
	if(cryptedPasswordHex.size() > 0) {
		std::vector<unsigned char> cryptedPassword = Utils::convertHexToData(cryptedPasswordHex);
		DesPasswordCipher("!_a^Rc*|#][Ych$~'(M _!d4aUo^%${T!~}h*&X%").decrypt(&cryptedPassword[0], (int)cryptedPassword.size());
		password = std::string(cryptedPassword.begin(), cryptedPassword.end());
	} else {
		password = thisInstance->password.get();
	}
/*
	std::vector<unsigned char> encryptedData = { 0x26, 0xd1, 0xfc, 0x23, 0x8c, 0x05, 0x26, 0x5c, 0xcd, 0x4f, 0xdc, 0x24, 0xa0, 0xdc, 0x21, 0x3b, 0xd9, 0xd8, 0xd8, 0xd8, 0xd1, 0x04, 0xed, 0x0e, 0xd8 };
	std::string result = ZlibCipher::decrypt(encryptedData);
	printf("Test: \"%s\"\n", result.c_str());
	*/

	thisInstance->connectionString.setDefault(
				"driver=" + thisInstance->driver.get() +
				";Server=" + thisInstance->server.get() +
				"," + std::to_string((long long)thisInstance->port.get()) +
				";Database=" + thisInstance->name.get() +
				";UID=" + thisInstance->account.get() +
				";PWD=" + password +
				";");
}
