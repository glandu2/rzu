#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"

namespace UploadServer {

struct ConnectionConfig {
	cval<std::string>& ip;
	cval<int>& port;

	ConnectionConfig(std::string prefix, int defaultPort, const char* defaultIp = "127.0.0.1") :
		ip(CFG_CREATE(prefix + ".ip", defaultIp)),
		port(CFG_CREATE(prefix + ".port", defaultPort))
	{}
};

struct GlobalConfig {

	ConnectionConfig upload;
	ConnectionConfig game;
	

	GlobalConfig() :
		upload("upload", 4617),
		game("game", 4616)
	{}

	static GlobalConfig* get();
	static void init();
};

} // namespace UploadServer

#ifndef CONFIG_GET
#define CONFIG_GET() UploadServer::GlobalConfig::get()
#endif

#endif // GLOBALCONFIG_H
