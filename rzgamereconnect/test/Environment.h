#pragma once

#include "TestEnvironment.h"

class Environment : public TestEnvironment {
public:
	virtual ~Environment() {}
	virtual void beforeTests();
	virtual void afterTests();
};

