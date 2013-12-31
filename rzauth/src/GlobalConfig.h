#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"

struct GlobalConfig {
	struct DbAccount {
		cval<std::string> &driver, &server, &name, &account, &password, &salt;
		cval<int> &port;

		DbAccount() :
			driver(CFG("db.driver", "FreeTDS")),
			server(CFG("db.server", "127.0.0.1")),
			name(CFG("db.name", "Auth")),
			account(CFG("db.account", "sa")),
			password(CFG("db.password", "")),
			salt(CFG("db.salt", "2012")),
			port(CFG("db.port", 1433)) {}
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

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
