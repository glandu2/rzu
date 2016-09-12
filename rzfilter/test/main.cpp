#include "RunTests.h"
#include "GlobalConfig.h"
#include "Environment.h"

static void initConfigs() {
	GlobalConfig::init();
}

int main(int argc, char **argv) {
	TestRunner testRunner(argc, argv, &initConfigs);
	::testing::AddGlobalTestEnvironment(new Environment);
	return testRunner.runTests();
}
