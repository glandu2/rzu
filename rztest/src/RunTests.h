#pragma once

#include "Extern.h"
#include "LibRzuInit.h"

typedef void (*ConfigInitCallback)();

class Log;

class RZTEST_EXTERN TestRunner {
public:
	TestRunner(int argc, char** argv, ConfigInitCallback configInit = nullptr);
	~TestRunner();

	int runTests();

private:
	Log* mainLogger;
	LibRzuScopedUse useLibRzu;
};

