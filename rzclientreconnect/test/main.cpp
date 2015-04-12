#include "RunTests.h"
#include "AuthServer/GlobalConfig.h"

static void initConfigs() {
	AuthServer::GlobalConfig::init();
}

int main(int argc, char **argv) {
	return runTests(argc, argv, &initConfigs);
}
