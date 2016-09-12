#include "Environment.h"
#include "AuthServer/GlobalConfig.h"

Environment* Environment::instance = nullptr;

void Environment::beforeTests() {
	std::string authExec = CONFIG_GET()->authExecutable.get();
	std::string gameReconnectExec = CONFIG_GET()->gameReconnectExecutable.get();

	spawnProcess(4500, authExec.c_str(), 1, "/configfile:auth-test.opt");
	if(testGameReconnect)
		spawnProcess(4802, gameReconnectExec.c_str(), 1, "/auth.reconnectdelay:100");
}

void Environment::afterTests() {
	stop(4501);
	if(testGameReconnect)
		stop(4801);
}

bool Environment::isGameReconnectBeingTested()
{
	if(instance && instance->testGameReconnect)
		return true;
	return false;
}
