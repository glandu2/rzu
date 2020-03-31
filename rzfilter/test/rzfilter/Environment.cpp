#include "Environment.h"
#include "GlobalConfig.h"

void Environment::beforeTests() {
	if(CONFIG_GET()->spawnRzfilter.get()) {
		std::string rzfilterExec = CONFIG_GET()->rzfilterExec.get();

		spawnProcess(4803,
		             rzfilterExec.c_str(),
		             5,
		             "/configfile:./rzfilter-test.opt",
		             ("/server.ip:" + CONFIG_GET()->server.ip.get()).c_str(),
		             ("/server.port:" + std::to_string(CONFIG_GET()->server.port.get())).c_str(),
		             ("/client.ip:" + CONFIG_GET()->client.ip.get()).c_str(),
		             ("/client.port:" + std::to_string(CONFIG_GET()->client.port.get())).c_str());
	}
}

void Environment::afterTests() {
	if(CONFIG_GET()->spawnRzfilter.get()) {
		stop(4803);
	}
}
