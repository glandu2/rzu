#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"
#include "Utils.h"

struct DbConfig : public IListener {
	cval<std::string> &driver, &server, &name, &account, &password, &cryptedPassword, &salt;
	cval<int> &port;
	cval<std::string> &connectionString, &cryptedConnectionString;
	cval<bool> &ignoreInitCheck;

	DbConfig(const std::string& prefix) :
		driver(CFG_CREATE(prefix + ".db.driver", "osdriver")), //Set in .cpp according to OS
		server(CFG_CREATE(prefix + ".db.server", "127.0.0.1")),
		name(CFG_CREATE(prefix + ".db.name", "Log")),
		account(CFG_CREATE(prefix + ".db.account", "sa", true)),
		password(CFG_CREATE(prefix + ".db.password", "", true)),
		cryptedPassword(CFG_CREATE(prefix + ".db.cryptedpassword", "", true)),
		salt(CFG_CREATE(prefix + ".db.salt", "2011")),
		port(CFG_CREATE(prefix + ".db.port", 1433)),
		connectionString(CFG_CREATE(prefix + ".db.connectionstring", "", true)),
		cryptedConnectionString(CFG_CREATE(prefix + ".db.cryptedconnectionstring", "", true)),
		ignoreInitCheck(CFG_CREATE(prefix + ".db.ignoreinitcheck", true))
	{
		driver.addListener(this, &updateConnectionString);
		server.addListener(this, &updateConnectionString);
		name.addListener(this, &updateConnectionString);
		account.addListener(this, &updateConnectionString);
		password.addListener(this, &updateConnectionString);
		cryptedPassword.addListener(this, &updateConnectionString);
		port.addListener(this, &updateConnectionString);
		cryptedConnectionString.addListener(this, &updateConnectionString);
		updateConnectionString(this);
	}

	static void updateConnectionString(IListener* instance);
};

struct ListenerConfig {
	cval<std::string> &listenIp;
	cval<int> &port, &idleTimeout;
	cval<bool> &autoStart;

	ListenerConfig(const std::string& prefix, const char* defaultIp, int defaultPort, bool autoStart = true, int idleTimeout = 0) :
		listenIp(CFG_CREATE(prefix + ".ip", defaultIp)),
		port(CFG_CREATE(prefix + ".port", defaultPort)),
		idleTimeout(CFG_CREATE(prefix + ".idletimeout", idleTimeout)),
		autoStart(CFG_CREATE(prefix + ".autostart", autoStart))
	{}
};

struct GlobalConfig {

	struct LogConfig {
		DbConfig db;

		struct ClientConfig {
			ListenerConfig listener;

			ClientConfig() :
				listener("log.clients", "127.0.0.1", 4516, true, 0) {}
		} client;

		LogConfig() :
			db("log") {}
	} log;

	struct AdminConfig {
		ListenerConfig listener;
		cval<int> &dumpMode;

		AdminConfig() :
			listener("admin.telnet", "127.0.0.1", 4517, true, 0),
			dumpMode(CFG_CREATE("admin.dump_mode", 0)) //1: no dump, anything else: create dump on crash
		{}
	} admin;

	struct TrafficDump {
		cval<bool> &enable;
		cval<std::string> &dir, &file, &level, &consoleLevel;

		TrafficDump() :
			enable(CFG_CREATE("trafficdump.enable", false)),
			dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
			file(CFG_CREATE("trafficdump.file", "rzlog.log")),
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
