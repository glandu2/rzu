#pragma once

#include "Config/ConfigInfo.h"

struct GlobalConfig {
	GlobalConfig() {}

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()
