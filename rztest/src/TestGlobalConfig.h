#pragma once

#include "Config/ConfigInfo.h"

struct TestGlobalConfig {
	cval<bool>& enableExecutableSpawn;

	TestGlobalConfig() : enableExecutableSpawn(CFG_CREATE("test.spawnexec", true)) {}

	static TestGlobalConfig* get();
	static void init();
};

