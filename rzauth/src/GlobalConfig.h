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
		driver(CFG_CREATE(prefix + "db.driver", "osdriver")), //Set in .cpp according to OS
		server(CFG_CREATE(prefix + "db.server", "127.0.0.1")),
		name(CFG_CREATE(prefix + "db.name", "Auth")),
		account(CFG_CREATE(prefix + "db.account", "sa")),
		password(CFG_CREATE(prefix + "db.password", "")),
		salt(CFG_CREATE(prefix + "db.salt", "2012")),
		port(CFG_CREATE(prefix + "db.port", 1433)),
		connectionString(CFG_CREATE(prefix + "db.connectionstring", "")),
		ignoreInitCheck(CFG_CREATE(prefix + "db.ignoreinitcheck", false))
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
			cval<bool> &autoStart;

			ClientConfig() :
				desKey(CFG_CREATE("auth.clients.des_key", "MERONG")),
				listenIp(CFG_CREATE("auth.clients.ip", "0.0.0.0")),
				port(CFG_CREATE("auth.clients.port", 4500)),
				autoStart(CFG_CREATE("auth.clients.autostart", true)) {}
		} client;

		struct GameConfig {
			cval<std::string> &listenIp;
			cval<int> &port;
			cval<bool> &autoStart, &strictKick;

			GameConfig() :
				listenIp(CFG_CREATE("auth.gameserver.ip", "127.0.0.1")),
				port(CFG_CREATE("auth.gameserver.port", 4502)),
				autoStart(CFG_CREATE("auth.gameserver.autostart", true)),
				strictKick(CFG_CREATE("auth.gameserver.strictkick", false)) {}
		} game;

		AuthConfig() :
			dbAccount("auth.") {}
	} auth;

	struct UploadConfig {
		struct ClientConfig {
			cval<std::string> &uploadDir, &listenIp;
			cval<int> &port, &webPort;
			cval<bool> &autoStart;

			ClientConfig() :
				uploadDir(CFG_CREATE("upload.dir", "upload")),
				listenIp(CFG_CREATE("upload.clients.ip", "0.0.0.0")),
				port(CFG_CREATE("upload.clients.port", 4617)),
				webPort(CFG_CREATE("upload.clients.webport", 80)),
				autoStart(CFG_CREATE("upload.clients.autostart", true))
			{
				Utils::autoSetAbsoluteDir(uploadDir);
			}
		} client;

		struct GameConfig {
			cval<std::string> &listenIp;
			cval<int> &port;
			cval<bool> &autoStart;

			GameConfig() :
				listenIp(CFG_CREATE("upload.gameserver.ip", "127.0.0.1")),
				port(CFG_CREATE("upload.gameserver.port", 4616)),
				autoStart(CFG_CREATE("upload.gameserver.autostart", true)) {}
		} game;
	} upload;

	struct AdminConfig {
		struct TelnetConfig {
			cval<std::string> &listenIp;
			cval<int> &port;
			cval<bool> &autoStart;

			TelnetConfig() :
				listenIp(CFG_CREATE("admin.telnet.ip", "127.0.0.1")),
				port(CFG_CREATE("admin.telnet.port", 4501)),
				autoStart(CFG_CREATE("admin.telnet.autostart", true))
			{}
		} telnet;

		cval<int> &dumpMode;

		AdminConfig() :
			dumpMode(CFG_CREATE("admin.dump_mode", 0)) //1: no dump, anything else: create dump on crash
		{}
	} admin;

	struct TrafficDump {
		cval<bool> &enable;
		cval<std::string> &dir, &file, &level, &consoleLevel;

		TrafficDump() :
			enable(CFG_CREATE("trafficdump.enable", false)),
			dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
			file(CFG_CREATE("trafficdump.file", "trafficdump.log")),
			level(CFG_CREATE("trafficdump.level", "debug")),
			consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal"))
		{
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	struct SqlQueries {
		struct DbAccount {
			cval<std::string>& query;
			cval<int> &paramAccount, &paramPassword;

			DbAccount() :
				query(CFG_CREATE("sql.db_account.query", "SELECT account_id FROM Account WHERE account = ? AND password = ?;")),
				paramAccount(CFG_CREATE("sql.db_account.param.account", 1)),
				paramPassword(CFG_CREATE("sql.db_account.param.password", 2)) {}
		} dbAccount;
	} sql;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
