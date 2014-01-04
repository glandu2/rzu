#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"

struct GlobalConfig {
	struct AuthConfig {
		struct DbAccount : public ICallbackGuard {
			cval<std::string> &driver, &server, &name, &account, &password, &salt;
			cval<int> &port;
			cval<std::string> &connectionString;
			cval<bool> &ignoreInitCheck;

			DbAccount() :
				driver(CFG("auth.db.driver", "osdriver")), //Set in .cpp according to OS
				server(CFG("auth.db.server", "127.0.0.1")),
				name(CFG("auth.db.name", "Auth")),
				account(CFG("auth.db.account", "sa")),
				password(CFG("auth.db.password", "")),
				salt(CFG("auth.db.salt", "2012")),
				port(CFG("auth.db.port", 1433)),
				connectionString(CFG("auth.db.connectionstring", "driver=" + driver.get() + ";Server=" + server.get() + ";Database=" + name.get() + ";UID=" + account.get() + ";PWD=" + password.get() + ";Port=" + std::to_string((long long)port.get()) + ";")),
				ignoreInitCheck(CFG("auth.db.ignoreinitcheck", false))
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
			cval<std::string> &desKey, &listenIp;
			cval<int> &port;

			ClientConfig() :
				desKey(CFG("auth.clients.des_key", "MERONG")),
				listenIp(CFG("auth.clients.ip", "0.0.0.0")),
				port(CFG("auth.clients.port", 4500)) {}
		} client;

		struct GameConfig {
			cval<std::string> &listenIp;
			cval<int> &port;

			GameConfig() :
				listenIp(CFG("auth.gameserver.ip", "0.0.0.0")),
				port(CFG("auth.gameserver.port", 4502)) {}
		} game;
	} auth;

	struct UploadConfig {
		struct ClientConfig {
			cval<std::string> &uploadDir, &listenIp;
			cval<int> &port, &webPort;

			ClientConfig() :
				uploadDir(CFG("upload.dir", "upload")),
				listenIp(CFG("upload.clients.ip", "0.0.0.0")),
				port(CFG("upload.clients.port", 4617)),
				webPort(CFG("upload.clients.webport", 80))  {}
		} client;

		struct GameConfig {
			cval<std::string> &listenIp;
			cval<int> &port;

			GameConfig() :
				listenIp(CFG("upload.gameserver.ip", "0.0.0.0")),
				port(CFG("upload.gameserver.port", 4616)) {}
		} game;
	} upload;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
