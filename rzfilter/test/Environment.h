#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "TestEnvironment.h"

class Environment : public TestEnvironment {
public:
	virtual ~Environment() {}
	virtual void beforeTests();
	virtual void afterTests();
};

#endif
