#ifndef RZUGTESTPRETTYUNITTESTRESULTPRINTER_H
#define RZUGTESTPRETTYUNITTESTRESULTPRINTER_H

#include "Core/Object.h"
#include "gtest/gtest.h"

class RzuTestPrinter : public Object, public testing::TestEventListener
{
public:

	// The following methods override what's in the TestEventListener class.
	virtual void OnTestProgramStart(const testing::UnitTest& /*unit_test*/) {}
	virtual void OnTestIterationStart(const testing::UnitTest& unit_test, int iteration);
	virtual void OnEnvironmentsSetUpStart(const testing::UnitTest& unit_test);
	virtual void OnEnvironmentsSetUpEnd(const testing::UnitTest& /*unit_test*/) {}
	virtual void OnTestCaseStart(const testing::TestCase& test_case);
	virtual void OnTestStart(const testing::TestInfo& test_info);
	virtual void OnTestPartResult(const testing::TestPartResult& result);
	virtual void OnTestEnd(const testing::TestInfo& test_info);
	virtual void OnTestCaseEnd(const testing::TestCase& test_case);
	virtual void OnEnvironmentsTearDownStart(const testing::UnitTest& unit_test);
	virtual void OnEnvironmentsTearDownEnd(const testing::UnitTest& /*unit_test*/) {}
	virtual void OnTestIterationEnd(const testing::UnitTest& unit_test, int iteration);
	virtual void OnTestProgramEnd(const testing::UnitTest& /*unit_test*/) {}

private:
	void PrintFailedTests(const testing::UnitTest& unit_test);
	std::string PrintFullTestCommentIfPresent(const testing::TestInfo& test_info);

	DECLARE_CLASS(RzuTestPrinter)
};

#endif // RZUGTESTPRETTYUNITTESTRESULTPRINTER_H

