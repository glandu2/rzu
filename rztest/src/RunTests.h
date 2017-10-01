#ifndef RUNTESTS_H
#define RUNTESTS_H

#include "Extern.h"

typedef void (*ConfigInitCallback)();

class Log;

class RZTEST_EXTERN TestRunner {
public:
	TestRunner(int argc, char** argv, ConfigInitCallback configInit = nullptr);
	~TestRunner();

	int runTests();

private:
	Log* mainLogger;
};

#endif
