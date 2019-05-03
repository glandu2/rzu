#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"
#include "Core/Utils.h"

struct ConnectionConfig {
	cval<std::string>& ip;
	cval<int>& port;

	ConnectionConfig(std::string prefix, int defaultPort, const char* defaultIp = "127.0.0.1")
	    : ip(CFG_CREATE(prefix + ".ip", defaultIp)), port(CFG_CREATE(prefix + ".port", defaultPort)) {}
};

struct GlobalConfig {
	ConnectionConfig client;
	ConnectionConfig server;
	ConnectionConfig gameServer;
	cval<int>& count;
	cval<std::string>& rzfilterExec;
	cval<bool>& spawnRzfilter;

	GlobalConfig()
	    : client("client", 4500),
	      server("server", 4800),
	      gameServer("gameServer", 4814, "127.0.0.2"),
	      count(CFG_CREATE("count", 10000)),
	      rzfilterExec(CFG_CREATE("rzfilter.exec", "rzfilter")),
	      spawnRzfilter(CFG_CREATE("rzfilter.spawn", true)) {
		Utils::autoSetAbsoluteDir(rzfilterExec);
	}

	static GlobalConfig* get();
	static void init();
};

#ifndef CONFIG_GET
#define CONFIG_GET() GlobalConfig::get()
#endif

#endif  // GLOBALCONFIG_H
