#pragma once

#include "Config/ConfigInfo.h"
#include "Core/Utils.h"

namespace AuthServer {

struct ConnectionConfig {
	cval<std::string>& ip;
	cval<int>& port;

	ConnectionConfig(std::string prefix, int defaultPort, const char* defaultIp = "127.0.0.1")
	    : ip(CFG_CREATE(prefix + ".ip", defaultIp)), port(CFG_CREATE(prefix + ".port", defaultPort)) {}
};

struct GlobalConfig {
	ConnectionConfig auth;
	ConnectionConfig game;
	ConnectionConfig billing;
	cval<std::string>& authExecutable;
	cval<std::string>& gameReconnectExecutable;
	cval<std::string>& connectionString;
	cval<bool>& doGameReconnectTest;

	GlobalConfig()
	    : auth("auth.clients", 4500),
	      game("auth.game", 4502),
	      billing("auth.billing", 4503),
	      authExecutable(CFG_CREATE("auth.exec", "rzauth")),
	      gameReconnectExecutable(CFG_CREATE("gamereconnect.exec", "rzgamereconnect")),
	      connectionString(CFG_CREATE("auth.db.connectionstring", "DRIVER={SQLite3 ODBC Driver};Database=rzauth.db;")),
	      doGameReconnectTest(CFG_CREATE("gamereconnect.enabletest", true))

	{
		Utils::autoSetAbsoluteDir(gameReconnectExecutable);
		Utils::autoSetAbsoluteDir(authExecutable);
	}

	static GlobalConfig* get();
	static void init();
};

auto inline CONFIG_GET() {
	return AuthServer::GlobalConfig::get();
}

}  // namespace AuthServer
