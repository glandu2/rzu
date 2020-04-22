#pragma once

#include "TestEnvironment.h"

class Environment : public TestEnvironment {
public:
	Environment() : testGameReconnect(false) { instance = this; }
	virtual ~Environment() {}
	virtual void beforeTests();
	virtual void afterTests();

	static bool isGameReconnectBeingTested();

	bool testGameReconnect;

private:
	static Environment* instance;
};

