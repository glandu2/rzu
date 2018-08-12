#include "BanManagerEnvironment.h"
#include "GlobalConfig.h"

void BanManagerEnvironment::beforeTests() {
	std::string rzfilterExec = CONFIG_GET()->rzfilterExec.get();

	spawnProcess(4803,
	             rzfilterExec.c_str(),
	             6,
	             "/configfile:./rzfilter-test.opt",
	             "/core.log.level:debug",
	             "/server.ip:127.0.0.1",
	             "/server.port:4800",
	             "/ban.maxconnectionperdayperip:4",
	             "/ban.maxclientperip:2");
}

void BanManagerEnvironment::afterTests() {
	stop(4803);
}
