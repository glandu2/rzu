#include "Extern.h"

typedef void (*ConfigInitCallback)();

RZTEST_EXTERN int runTests(int argc, char **argv, ConfigInitCallback configInit = nullptr);
