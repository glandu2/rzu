#include "BanManagerEnvironment.h"
#include "GlobalConfig.h"
#include "RunTests.h"

static void initConfigs() {
	GlobalConfig::init();
}

int main(int argc, char** argv) {
	TestRunner testRunner(argc, argv, &initConfigs);
	::testing::AddGlobalTestEnvironment(new BanManagerEnvironment);
	return testRunner.runTests();
}
