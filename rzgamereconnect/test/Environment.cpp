#include "Environment.h"
#include "AuthServer/GlobalConfig.h"

void Environment::beforeTests() {
	std::string gameReconnectExec = CONFIG_GET()->gameReconnectExec.get();

	spawnProcess(4802, gameReconnectExec.c_str(), 1, "/auth.reconnectdelay:100");
}

void Environment::afterTests() {
	stop(4801);
}
