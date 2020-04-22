#pragma once

#include "TestEnvironment.h"

class BanManagerEnvironment : public TestEnvironment {
public:
	virtual ~BanManagerEnvironment() {}
	virtual void beforeTests();
	virtual void afterTests();
};

