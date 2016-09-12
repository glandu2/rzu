#ifndef TESTENVIRONMENT_H
#define TESTENVIRONMENT_H

#include "gtest/gtest.h"
#include "Core/Object.h"
#include "Extern.h"
#include "uv.h"

class RZTEST_EXTERN TestEnvironment : public ::testing::Environment, public Object
{
	DECLARE_CLASS(TestEnvironment)
public:
	virtual ~TestEnvironment();
	// Override this to define how to set up the environment.
	virtual void SetUp();
	// Override this to define how to tear down the environment.
	virtual void TearDown();

protected:
	virtual void beforeTests() = 0;
	virtual void afterTests() = 0;


protected:
	void spawnProcess(int portCheck, const char* exeFile, int argNumber, ...);
	void stop(int port);

private:
	static void executableExited(uv_process_t* process, int64_t exit_status, int);
	void tearDownTimeout();

	std::vector<uv_process_t*> processes;
	bool doKillAll;
};

#endif
