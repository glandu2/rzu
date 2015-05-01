#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"

struct ConnectionConfig {
	cval<std::string>& ip;
	cval<int>& port;

	ConnectionConfig(std::string prefix, int defaultPort, const char* defaultIp = "127.0.0.1") :
		ip(CFG_CREATE(prefix + ".ip", defaultIp)),
		port(CFG_CREATE(prefix + ".port", defaultPort))
	{}
};

struct GlobalConfig {
	ConnectionConfig input;
	ConnectionConfig output;
	cval<int>& count;

	GlobalConfig() :
		input("input", 4500),
		output("output", 4800),
		count(CFG_CREATE("count", 10000))
	{}

	static GlobalConfig* get();
	static void init();
};

#ifndef CONFIG_GET
#define CONFIG_GET() GlobalConfig::get()
#endif

#endif // GLOBALCONFIG_H
