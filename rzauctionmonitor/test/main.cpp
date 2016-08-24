#include "RunTests.h"
#include "GlobalConfig.h"

static void initConfigs() {
	GlobalConfig::init();
}

int main(int argc, char **argv) {
	return runTests(argc, argv, &initConfigs);
}
