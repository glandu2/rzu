#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"

namespace AuthServer {

struct ConnectionConfig {
	cval<std::string>& ip;
	cval<int>& port;

	ConnectionConfig(std::string prefix, int defaultPort, const char* defaultIp = "127.0.0.1") :
		ip(CFG_CREATE(prefix + ".ip", defaultIp)),
		port(CFG_CREATE(prefix + ".port", defaultPort))
	{}
};

struct GlobalConfig {

	ConnectionConfig auth;
	ConnectionConfig game;

	GlobalConfig() :
		auth("auth", 4502, "0.0.0.0"),
		game("game", 4802)
	{}

	static GlobalConfig* get();
	static void init();
};

} // namespace AuthServer

#ifndef CONFIG_GET
#define CONFIG_GET() AuthServer::GlobalConfig::get()
#endif

#endif // GLOBALCONFIG_H
