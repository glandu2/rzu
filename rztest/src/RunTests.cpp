#include "RunTests.h"
#include "gtest/gtest.h"
#include "LibRzuInit.h"
#include "Core/Log.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/Log.h"
#include "RzuGtestPrettyUnitTestResultPrinter.h"

int runTests(int argc, char **argv, ConfigInitCallback configInit) {

	LibRzuInit();
	if(configInit)
		configInit();

	ConfigInfo::get()->init(argc, argv);
	Log mainLogger(GlobalCoreConfig::get()->log.enable,
				   GlobalCoreConfig::get()->log.level,
				   GlobalCoreConfig::get()->log.consoleLevel,
				   GlobalCoreConfig::get()->log.dir,
				   GlobalCoreConfig::get()->log.file,
				   GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);
	ConfigInfo::get()->dump();

	testing::InitGoogleTest(&argc, argv);

	testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
	delete listeners.Release(listeners.default_result_printer());
	listeners.Append(new RzuTestPrinter);

	return RUN_ALL_TESTS();
}
