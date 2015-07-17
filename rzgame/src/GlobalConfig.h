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
		name(CFG_CREATE(prefix + "db.name", "Telecaster")),
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
	struct AdminConfig {
		struct TelnetConfig {
			cval<std::string> &listenIp;
			cval<int> &port;
			cval<bool> &autoStart;

			TelnetConfig() :
				listenIp(CFG_CREATE("admin.telnet.ip", "127.0.0.1")),
				port(CFG_CREATE("admin.telnet.port", 4515)),
				autoStart(CFG_CREATE("admin.telnet.autostart", true))
			{}
		} telnet;

		cval<int> &dumpMode;

		AdminConfig() :
			dumpMode(CFG_CREATE("admin.dump_mode", 0)) //1: no dump, anything else: create dump on crash
		{}
	} admin;

	struct GameConfig {
		struct ClientsConfig {
			cval<std::string> &listenIp;
			cval<int> &port;
			cval<bool> &autoStart;

			ClientsConfig() :
				listenIp(CFG_CREATE("game.clients.ip", "127.0.0.1")),
				port(CFG_CREATE("game.clients.port", 4514)),
				autoStart(CFG_CREATE("game.clients.autostart", true))
			{}
		} clients;
	} game;

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

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif // GLOBALCONFIG_H
