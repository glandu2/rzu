#pragma once

#ifndef _WIN32
#include <unistd.h>
#define tfMicroSleep(i) usleep(i)
#else
#include <windows.h>
#define tfMicroSleep(i) \
	timeBeginPeriod(1); \
	Sleep((i) / 1000); \
	timeEndPeriod(1);
#endif

