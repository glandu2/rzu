#include "RunTests.h"
#include "gtest/gtest.h"
#include "LibRzuInit.h"
#include "Core/Log.h"
#include "Config/GlobalCoreConfig.h"
#include "TestGlobalConfig.h"
#include "RzuGtestPrettyUnitTestResultPrinter.h"

TestRunner::TestRunner(int argc, char **argv, ConfigInitCallback configInit) {
	LibRzuInit();
	TestGlobalConfig::init();
	if(configInit)
		configInit();

	ConfigInfo::get()->init(argc, argv);
	mainLogger = new Log(GlobalCoreConfig::get()->log.enable,
				   GlobalCoreConfig::get()->log.level,
				   GlobalCoreConfig::get()->log.consoleLevel,
				   GlobalCoreConfig::get()->log.dir,
				   GlobalCoreConfig::get()->log.file,
				   GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(mainLogger);
	ConfigInfo::get()->dump();

	testing::InitGoogleTest(&argc, argv);

	testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
	delete listeners.Release(listeners.default_result_printer());
	listeners.Append(new RzuTestPrinter);
}

TestRunner::~TestRunner()
{
	delete mainLogger;
}

int TestRunner::runTests()
{
	return RUN_ALL_TESTS();
}
