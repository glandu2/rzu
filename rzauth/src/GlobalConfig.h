#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"

struct GlobalConfig {
	struct {
		std::string& driver = CFG("db.driver", "FreeTDS");
		std::string& server = CFG("db.server", "127.0.0.1");
		int& port = CFG("db.port", 1433);
		std::string& name = CFG("db.name", "Auth");
		std::string& account = CFG("db.account", "sa");
		std::string& password = CFG("db.password", "");
		std::string& salt = CFG("db.salt", "2012");
	} dbAccount;

	struct {
		std::string& listenIp = CFG("listen.client.ip", "0.0.0.0");
		int& port = CFG("listen.client.port", 4500);
		std::string& desKey = CFG("listen.client.des_key", "MERONG");
	} clientConfig;

	struct {
		std::string& listenIp = CFG("listen.game.ip", "0.0.0.0");
		int& port = CFG("listen.game.port", 4502);
	} gameConfig;

	static GlobalConfig* get();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
