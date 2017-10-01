#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"

struct GlobalConfig {
	GlobalConfig() {}

	static GlobalConfig* get();
	static void init();
};

#ifndef CONFIG_GET
#define CONFIG_GET() GlobalConfig::get()
#endif

#endif  // GLOBALCONFIG_H
