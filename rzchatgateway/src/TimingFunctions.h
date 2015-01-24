#ifndef TIMINGFUNCTIONS_H
#define TIMINGFUNCTIONS_H

#ifdef __linux__
	#include <unistd.h>
	#define tfMicroSleep(i) usleep(i)
#else
	#include <windows.h>
	#define tfMicroSleep(i) timeBeginPeriod(1); \
		Sleep((i)/1000);  \
		timeEndPeriod(1);
#endif

unsigned long long timingFunction_ucounter;

#if defined(__linux__)

	#include <time.h>
	#ifndef CLOCK_MONOTONIC_RAW
		#define CLOCK_MONOTONIC_RAW 4
	#endif

	inline void initTimer() {
	}

	void resetTimer() {
		struct timespec clk;
		clock_gettime(CLOCK_MONOTONIC_RAW, &clk);
		timingFunction_ucounter = clk.tv_sec*1000000 + clk.tv_nsec/1000;
	}

	unsigned long long getTimerValue() {
		struct timespec clk;
		unsigned long long uclk;

		clock_gettime(CLOCK_MONOTONIC_RAW, &clk);
		uclk = clk.tv_sec*1000000 + clk.tv_nsec/1000;
		return uclk - timingFunction_ucounter;
	}
#else
	LARGE_INTEGER clockFreq;
	void initTimer() {
		QueryPerformanceFrequency(&clockFreq);
	}

	void resetTimer() {
		LARGE_INTEGER clk;
		QueryPerformanceCounter(&clk);
		timingFunction_ucounter = clk.QuadPart;
	}

	unsigned long long getTimerValue() {
		LARGE_INTEGER clk;
		QueryPerformanceCounter(&clk);
		return (clk.QuadPart - timingFunction_ucounter)*1000000/clockFreq.QuadPart;
	}
#endif

#endif // TIMINGFUNCTIONS_H
