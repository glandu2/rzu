#include "RunTests.h"
#include "AuthServer/GlobalConfig.h"
#include "gtest/gtest.h"
#include "Environment.h"

static void initConfigs() {
	AuthServer::GlobalConfig::init();
}

int main(int argc, char **argv) {
	TestRunner testRunner(argc, argv, &initConfigs);
	Environment* env = new Environment;

	::testing::AddGlobalTestEnvironment(env);

	env->testGameReconnect = false;
	int result = testRunner.runTests();
	if(result)
		return result;

	env->testGameReconnect = true;
	return testRunner.runTests();
}
