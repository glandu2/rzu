#include "RunTests.h"
#include "GlobalConfig.h"

static void initConfigs() {
	GlobalConfig::init();
}

int main(int argc, char **argv) {
	TestRunner testRunner(argc, argv, &initConfigs);
	return testRunner.runTests();
}
