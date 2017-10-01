#include "GlobalConfig.h"
#include "RunTests.h"

static void initConfigs() {
	GlobalConfig::init();
}

int main(int argc, char** argv) {
	TestRunner testRunner(argc, argv, &initConfigs);
	return testRunner.runTests();
}
