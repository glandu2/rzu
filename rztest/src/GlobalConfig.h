#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "ConfigInfo.h"
#include "Extern.h"

struct RZTEST_EXTERN GlobalTestConfig {

	struct AuthConfig {
		cval<std::string>& ip;
		cval<int>& port;

		AuthConfig() :
			ip(CFG_CREATE("auth.ip", "127.0.0.1")),
			port(CFG_CREATE("auth.port", 4500))
		{}
	} auth;

	static GlobalTestConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalTestConfig::get()

#endif // GLOBALCONFIG_H
