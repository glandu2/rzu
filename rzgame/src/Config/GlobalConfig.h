#pragma once

#include "Config/ConfigInfo.h"
#include "Core/Utils.h"
#include "Packet/PacketEpics.h"

#ifdef _WIN32
#define DEFAULT_SQL_DRIVER "SQL Server"
#else
#define DEFAULT_SQL_DRIVER "FreeTDS"
#endif

struct DbConfig : public IListener {
	cval<std::string>&driver, &server, &name, &account, &password, &cryptedPassword, &salt;
	cval<int>& port;
	cval<std::string>&connectionString, &cryptedConnectionString;
	cval<bool>& ignoreInitCheck;

	DbConfig(const std::string& prefix, const char* defaultDatabase)
	    : driver(CFG_CREATE(prefix + ".driver", DEFAULT_SQL_DRIVER)),
	      server(CFG_CREATE(prefix + ".server", "127.0.0.1")),
	      name(CFG_CREATE(prefix + ".name", defaultDatabase)),
	      account(CFG_CREATE(prefix + ".account", "sa", true)),
	      password(CFG_CREATE(prefix + ".password", "", true)),
	      cryptedPassword(CFG_CREATE(prefix + ".cryptedpassword", "", true)),
	      salt(CFG_CREATE(prefix + ".salt", "2011")),
	      port(CFG_CREATE(prefix + ".port", 1433)),
	      connectionString(CFG_CREATE(prefix + ".connectionstring", "", true)),
	      cryptedConnectionString(CFG_CREATE(prefix + ".cryptedconnectionstring", "", true)),
	      ignoreInitCheck(CFG_CREATE(prefix + ".ignoreinitcheck", true)) {
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

struct GlobalConfig {
	struct GameConfig {
		DbConfig arcadia;     // Reference data
		DbConfig telecaster;  // Player data
		cval<std::string>& urlList;

		struct ClientsConfig {
			cval<std::string>& listenIp;
			cval<int>&port, &idleTimeout;
			cval<bool>& autoStart;
			cval<int>& epic;

			ClientsConfig()
			    : listenIp(CFG_CREATE("game.clients.ip", "127.0.0.1")),
			      port(CFG_CREATE("game.clients.port", 4514)),
			      idleTimeout(CFG_CREATE("game.clients.idletimeout", 300)),
			      autoStart(CFG_CREATE("game.clients.autostart", true)),
			      epic(CFG_CREATE("game.clients.epic", EPIC_9_1)) {}
		} clients;

		GameConfig()
		    : arcadia("game.db.arcadia", "Arcadia"),
		      telecaster("game.db.telecaster", "Telecaster"),
		      urlList(CFG_CREATE("game.url_list",
		                         "guild_icon_upload.ip|127.0.0.1|guild_icon_upload.port|4617|guild_test_download.url|"
		                         "upload/|web_download|127.0.0.1:80")) {}
	} game;

	struct TrafficDump {
		cval<bool>& enable;
		cval<std::string>&dir, &file, &level, &consoleLevel;

		TrafficDump()
		    : enable(CFG_CREATE("trafficdump.enable", false)),
		      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		      file(CFG_CREATE("trafficdump.file", "game.log")),
		      level(CFG_CREATE("trafficdump.level", "debug")),
		      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal")) {
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

