#include "AuthServer/GlobalConfig.h"
#include "Environment.h"
#include "RunTests.h"
#include "gtest/gtest.h"

static void initConfigs() {
	AuthServer::GlobalConfig::init();
}

int main(int argc, char** argv) {
	TestRunner testRunner(argc, argv, &initConfigs);
	Environment* env = new Environment;

	// gtest takes ownership
	::testing::AddGlobalTestEnvironment(env);

	env->testGameReconnect = false;
	int result = testRunner.runTests();
	if(result)
		return result;

	if(CONFIG_GET()->doGameReconnectTest.get()) {
		env->testGameReconnect = true;
		return testRunner.runTests();
	} else {
		return result;
	}
}
