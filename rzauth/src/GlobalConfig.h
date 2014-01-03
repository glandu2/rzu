#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"

struct GlobalConfig {
	struct DbAccount : public ICallbackGuard {
		cval<std::string> &driver, &server, &name, &account, &password, &salt;
		cval<int> &port;
		cval<std::string> &connectionString;
		cval<bool> &ignoreInitCheck;

		DbAccount() :
			driver(CFG("db.driver", "osdriver")), //Set in .cpp according to OS
			server(CFG("db.server", "127.0.0.1")),
			name(CFG("db.name", "Auth")),
			account(CFG("db.account", "sa")),
			password(CFG("db.password", "")),
			salt(CFG("db.salt", "2012")),
			port(CFG("db.port", 1433)),
			connectionString(CFG("db.connectionstring", "driver=" + driver.get() + ";Server=" + server.get() + ";Database=" + name.get() + ";UID=" + account.get() + ";PWD=" + password.get() + ";Port=" + std::to_string((long long)port.get()) + ";")),
			ignoreInitCheck(CFG("db.ignoreinitcheck", false))
		{
			driver.addListener(this, &updateConnectionString);
			server.addListener(this, &updateConnectionString);
			name.addListener(this, &updateConnectionString);
			account.addListener(this, &updateConnectionString);
			password.addListener(this, &updateConnectionString);
			port.addListener(this, &updateConnectionString);
		}

		static void updateConnectionString(ICallbackGuard* instance);
	} dbAccount;

	struct ClientConfig {
		cval<std::string> &listenIp, &desKey;
		cval<int> &port;

		ClientConfig() :
			listenIp(CFG("listen.client.ip", "0.0.0.0")),
			desKey(CFG("listen.client.des_key", "MERONG")),
			port(CFG("listen.client.port", 4500)) {}
	} clientConfig;

	struct GameConfig {
		cval<std::string> &listenIp;
		cval<int> &port;

		GameConfig() :
			listenIp(CFG("listen.game.ip", "0.0.0.0")),
			port(CFG("listen.game.port", 4502)) {}
	} gameConfig;


	struct UploadClientConfig {
		cval<std::string> &uploadDir, &listenIp;
		cval<int> &port;

		UploadClientConfig() :
			uploadDir(CFG("upload.dir", "upload")),
			listenIp(CFG("listen.upload.client.ip", "0.0.0.0")),
			port(CFG("listen.upload.client.port", 4617)) {}
	} uploadClientConfig;

	struct UploadGameConfig {
		cval<std::string> &listenIp;
		cval<int> &port;

		UploadGameConfig() :
			listenIp(CFG("listen.upload.game.ip", "0.0.0.0")),
			port(CFG("listen.upload.game.port", 4616)) {}
	} uploadGameConfig;
	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
