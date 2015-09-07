#include "RzuGtestPrettyUnitTestResultPrinter.h"
#include "Core/Utils.h"
#include <string.h>

// Text printed in Google Test's text output and --gunit_list_tests
// output to label the type parameter and value parameter for a test.
static const char kTypeParamLabel[] = "TypeParam";
static const char kValueParamLabel[] = "GetParam()";

std::string RzuTestPrinter::PrintFullTestCommentIfPresent(const testing::TestInfo& test_info) {
	const char* const type_param = test_info.type_param();
	const char* const value_param = test_info.value_param();

	if (type_param != NULL || value_param != NULL) {
		std::string typeComment;
		std::string valueComment;

		if (type_param != NULL) {
			Utils::stringFormat(typeComment, "%s = %s", kTypeParamLabel, type_param);
			if (value_param != NULL)
				typeComment += " and ";
		}
		if (value_param != NULL) {
			Utils::stringFormat(valueComment, "%s = %s", kValueParamLabel, value_param);
		}
		return ", where " + typeComment + valueComment;
	}

	return std::string();
}

// Fired before each iteration of tests starts.
void RzuTestPrinter::OnTestIterationStart(
		const testing::UnitTest& unit_test, int iteration) {
	if (iteration > 0)
		info("Repeating all tests (iteration %d) . . .\n", iteration + 1);

	const char* const filter = testing::GTEST_FLAG(filter).c_str();

	// Prints the filter if it's not *.  This reminds the user that some
	// tests may be skipped.
	if (strcmp(filter, "*") != 0) {
		info("Note: %s filter = %s\n", GTEST_NAME_, filter);
	}

	if (testing::GTEST_FLAG(shuffle)) {
		info("Note: Randomizing tests' orders with a seed of %d .\n",
			 unit_test.random_seed());
	}

	info("[==========] Running %d tests from %d test cases.\n",
		 unit_test.test_to_run_count(),
		 unit_test.test_case_to_run_count());
}

void RzuTestPrinter::OnEnvironmentsSetUpStart(const testing::UnitTest& /*unit_test*/) {
	info("[----------] Global test environment set-up.\n");
}

void RzuTestPrinter::OnTestCaseStart(const testing::TestCase& test_case) {
	if (test_case.type_param() == NULL) {
		info("[----------] %d tests from %s\n", test_case.test_to_run_count(), test_case.name());
	} else {
		info("[----------] %d tests from %s, where %s = %s\n", test_case.test_to_run_count(), test_case.name(), kTypeParamLabel, test_case.type_param());
	}
}

void RzuTestPrinter::OnTestStart(const testing::TestInfo& test_info) {
	info("[ RUN      ] %s.%s\n", test_info.test_case_name(), test_info.name());
}

static const char* getResultTypeName(testing::TestPartResult::Type type) {
	switch (type) {
		case testing::TestPartResult::kSuccess:
			return "Success";

		case testing::TestPartResult::kNonFatalFailure:
		case testing::TestPartResult::kFatalFailure:
			return "Failure";
	}
	return "Unknown result type";
}

// Called after an assertion failure.
void RzuTestPrinter::OnTestPartResult(const testing::TestPartResult& result) {
	// If the test part succeeded, we don't need to do anything.
	if (result.type() == testing::TestPartResult::kSuccess)
		return;

	// Print failure message from the assertion (e.g. expected this and got that).
	if(result.file_name()) {
		info("%s(%d): %s: %s\n",
			 result.file_name(),
			 result.line_number(),
			 getResultTypeName(result.type()),
			 result.message());
	} else {
		info("%s: %s\n",
			 getResultTypeName(result.type()),
			 result.message());
	}
}

void RzuTestPrinter::OnTestEnd(const testing::TestInfo& test_info) {
	const char* result = "[  FAILED  ] ";
	if (test_info.result()->Passed()) {
		result = "[       OK ] ";
	}
	std::string comment;
	if (test_info.result()->Failed())
		comment = PrintFullTestCommentIfPresent(test_info);

	if (testing::GTEST_FLAG(print_time)) {
		info("%s%s.%s%s (%d ms)\n", result, test_info.test_case_name(), test_info.name(), comment.c_str(), (int)test_info.result()->elapsed_time());
	} else {
		info("%s%s.%s%s\n", result, test_info.test_case_name(), test_info.name(), comment.c_str());
	}
}

void RzuTestPrinter::OnTestCaseEnd(const testing::TestCase& test_case) {
	if (!testing::GTEST_FLAG(print_time)) return;

	info("[----------] %d tests from %s (%d ms total)\n",
		 test_case.test_to_run_count(),
		 test_case.name(),
		 (int)test_case.elapsed_time());
}

void RzuTestPrinter::OnEnvironmentsTearDownStart(
		const testing::UnitTest& /*unit_test*/) {
	info("[----------] Global test environment tear-down\n");
}

// Internal helper for printing the list of failed tests.
void RzuTestPrinter::PrintFailedTests(const testing::UnitTest& unit_test) {
	const int failed_test_count = unit_test.failed_test_count();
	if (failed_test_count == 0) {
		return;
	}

	for (int i = 0; i < unit_test.total_test_case_count(); ++i) {
		const testing::TestCase& test_case = *unit_test.GetTestCase(i);
		if (!test_case.should_run() || (test_case.failed_test_count() == 0)) {
			continue;
		}
		for (int j = 0; j < test_case.total_test_count(); ++j) {
			const testing::TestInfo& test_info = *test_case.GetTestInfo(j);
			if (!test_info.should_run() || test_info.result()->Passed()) {
				continue;
			}
			info("[  FAILED  ] %s.%s%s\n",
				 test_case.name(),
				 test_info.name(),
				 PrintFullTestCommentIfPresent(test_info).c_str());
		}
	}
}

void RzuTestPrinter::OnTestIterationEnd(const testing::UnitTest& unit_test,
										int /*iteration*/) {
	if (testing::GTEST_FLAG(print_time)) {
		info("[==========] %d tests from %d test cases ran. (%d ms total)\n",
			 unit_test.test_to_run_count(),
			 unit_test.test_case_to_run_count(),
			 (int)unit_test.elapsed_time());
	} else {
		info("[==========] %d tests from %d test cases ran.", unit_test.test_to_run_count(), unit_test.test_case_to_run_count());
	}
	info("[  PASSED  ] %d tests.\n", unit_test.successful_test_count());

	int num_failures = unit_test.failed_test_count();
	if (!unit_test.Passed()) {
		const int failed_test_count = unit_test.failed_test_count();
		info("[  FAILED  ] %d tests, listed below:\n", failed_test_count);
		PrintFailedTests(unit_test);
		info("\n%2d FAILED %s\n", num_failures,
			 num_failures == 1 ? "TEST" : "TESTS");
	}

	int num_disabled = unit_test.reportable_disabled_test_count();
	if (num_disabled && !testing::GTEST_FLAG(also_run_disabled_tests)) {
		info("  YOU HAVE %d DISABLED %s\n",
			 num_disabled,
			 num_disabled == 1 ? "TEST" : "TESTS");
	}
}

// End PrettyUnitTestResultPrinter
