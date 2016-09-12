#ifndef TESTGLOBALCONFIG_H
#define TESTGLOBALCONFIG_H

#include "Config/ConfigInfo.h"

struct TestGlobalConfig {
	cval<bool> &enableExecutableSpawn;

	TestGlobalConfig() :
	    enableExecutableSpawn(CFG_CREATE("test.spawnexec", true))
	{
	}

	static TestGlobalConfig* get();
	static void init();
};

#endif // TESTGLOBALCONFIG_H
