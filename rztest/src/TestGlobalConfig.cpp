#include "TestGlobalConfig.h"
#include "rztestGitVersion.h"

TestGlobalConfig* TestGlobalConfig::get() {
	static TestGlobalConfig config;
	return &config;
}

void TestGlobalConfig::init() {
	TestGlobalConfig::get();
	CFG_CREATE("test.version", rztestVersion);
}
