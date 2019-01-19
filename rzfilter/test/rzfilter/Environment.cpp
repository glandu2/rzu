#include "Environment.h"
#include "GlobalConfig.h"

void Environment::beforeTests() {
	if(CONFIG_GET()->spawnRzfilter.get()) {
		std::string rzfilterExec = CONFIG_GET()->rzfilterExec.get();

		spawnProcess(4500,
		             rzfilterExec.c_str(),
		             3,
		             "/configfile:./rzfilter-test.opt",
		             "/server.ip:127.0.0.1",
		             "/server.port:4800");
	}
}

void Environment::afterTests() {
	if(CONFIG_GET()->spawnRzfilter.get()) {
		stop(4803);
	}
}
