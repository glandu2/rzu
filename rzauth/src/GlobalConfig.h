#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"
#include "Utils.h"

struct DbConfig : public IListener {
	cval<std::string> &driver, &server, &name, &account, &password, &salt;
	cval<int> &port;
	cval<std::string> &connectionString;
	cval<bool> &ignoreInitCheck;

	DbConfig(const std::string& prefix) :
		driver(CFG_STR(prefix + "db.driver", "osdriver")), //Set in .cpp according to OS
		server(CFG_STR(prefix + "db.server", "127.0.0.1")),
		name(CFG_STR(prefix + "db.name", "Auth")),
		account(CFG_STR(prefix + "db.account", "sa")),
		password(CFG_STR(prefix + "db.password", "")),
		salt(CFG_STR(prefix + "db.salt", "2012")),
		port(CFG_STR(prefix + "db.port", 1433)),
		connectionString(CFG_STR(prefix + "db.connectionstring", "")),
		ignoreInitCheck(CFG_STR(prefix + "db.ignoreinitcheck", false))
	{
		driver.addListener(this, &updateConnectionString);
		server.addListener(this, &updateConnectionString);
		name.addListener(this, &updateConnectionString);
		account.addListener(this, &updateConnectionString);
		password.addListener(this, &updateConnectionString);
		port.addListener(this, &updateConnectionString);
		updateConnectionString(this);
	}

	static void updateConnectionString(IListener* instance);
};

struct GlobalConfig {

	struct AuthConfig {
		DbConfig dbAccount;

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

		AuthConfig() : dbAccount("auth.") {}
	} auth;

	struct UploadConfig {
		struct ClientConfig {
			cval<std::string> &uploadDir, &listenIp;
			cval<int> &port, &webPort;

			ClientConfig() :
				uploadDir(CFG("upload.dir", "upload")),
				listenIp(CFG("upload.clients.ip", "0.0.0.0")),
				port(CFG("upload.clients.port", 4617)),
				webPort(CFG("upload.clients.webport", 80))
			{
				Utils::autoSetAbsoluteDir(uploadDir);
			}
		} client;

		struct GameConfig {
			cval<std::string> &listenIp;
			cval<int> &port;

			GameConfig() :
				listenIp(CFG("upload.gameserver.ip", "0.0.0.0")),
				port(CFG("upload.gameserver.port", 4616)) {}
		} game;
	} upload;

	struct Ban {
		DbConfig dbBan;
		cval<std::string> &banFile;

		Ban() :
			dbBan("ban."),
			banFile(CFG("ban.ipfile", "bannedip.txt"))
		{
			Utils::autoSetAbsoluteDir(banFile);
		}
	} ban;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
