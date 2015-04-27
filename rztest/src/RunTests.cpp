#include "RunTests.h"
#include "gtest.h"
#include "LibRzuInit.h"
#include "Log.h"
#include "GlobalCoreConfig.h"

int runTests(int argc, char **argv, ConfigInitCallback configInit) {

  LibRzuInit();
  if(configInit)
	  configInit();

  ConfigInfo::get()->init(argc, argv);
  ConfigInfo::get()->dump();

  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
