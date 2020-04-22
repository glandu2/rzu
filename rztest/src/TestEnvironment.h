#pragma once

#include "Core/Object.h"
#include "Extern.h"
#include "uv.h"
#include "gtest/gtest.h"
#include <stdarg.h>

class RZTEST_EXTERN TestProcessBase : public Object {
	DECLARE_CLASS(TestProcessBase)
public:
	virtual ~TestProcessBase();
	// Override this to define how to set up the environment.
	void SetUp();
	// Override this to define how to tear down the environment.
	void TearDown();

	void vspawnProcess(int portCheck, const char* exeFile, int argNumber, va_list args);
	void spawnProcess(int portCheck, const char* exeFile, int argNumber, ...);
	void stop(int port);

private:
	static void executableExited(uv_process_t* process, int64_t exit_status, int);
	void tearDownTimeout();

	std::vector<uv_process_t*> processes;
	bool doKillAll;
};

class RZTEST_EXTERN TestEnvironment : public TestProcessBase, public ::testing::Environment {
	DECLARE_CLASSNAME(TestEnvironment, 0)
public:
	virtual ~TestEnvironment();
	// Override this to define how to set up the environment.
	virtual void SetUp() {
		TestProcessBase::SetUp();
		beforeTests();
	}
	// Override this to define how to tear down the environment.
	virtual void TearDown() {
		afterTests();
		TestProcessBase::TearDown();
	}

protected:
	virtual void beforeTests() = 0;
	virtual void afterTests() = 0;
};

template<class Derived> class TestFixture : public ::testing::Test {
	DECLARE_CLASSNAME(TestFixture, 0)
public:
	// Override this to define how to set up the environment.
	static void SetUpTestCase() {
		testProcessBase.SetUp();
		Derived::beforeTests();
	}
	// Override this to define how to tear down the environment.
	static void TearDownTestCase() {
		Derived::afterTests();
		testProcessBase.TearDown();
	}

protected:
	static void spawnProcess(int portCheck, const char* exeFile, int argNumber, ...) {
		va_list argsList;
		va_start(argsList, argNumber);
		testProcessBase.vspawnProcess(portCheck, exeFile, argNumber, argsList);
		va_end(argsList);
	}
	static void stop(int port) { testProcessBase.stop(port); }

private:
	static TestProcessBase testProcessBase;
};

template<class Derived> TestProcessBase TestFixture<Derived>::testProcessBase;

