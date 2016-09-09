#include "RunTests.h"
#include "AuthServer/GlobalConfig.h"
#include "Environment.h"

static void initConfigs() {
	AuthServer::GlobalConfig::init();
}

int main(int argc, char **argv) {
	TestRunner testRunner(argc, argv, &initConfigs);
	::testing::AddGlobalTestEnvironment(new Environment);
	return testRunner.runTests();
}
