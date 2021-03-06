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
	cval<std::string>& gameReconnectExec;

	GlobalConfig()
	    : auth("auth", 4502, "0.0.0.0"),
	      game("game", 4802),
	      gameReconnectExec(CFG_CREATE("gamereconnect.exec", "rzgamereconnect")) {
		Utils::autoSetAbsoluteDir(gameReconnectExec);
	}

	static GlobalConfig* get();
	static void init();
};

}  // namespace AuthServer

#define CONFIG_GET() AuthServer::GlobalConfig::get()
