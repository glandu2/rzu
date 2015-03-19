#include "gtest.h"
#include "LibRzuInit.h"
#include "Log.h"
#include "GlobalConfig.h"
#include "GlobalCoreConfig.h"
#include "Extern.h"

RZTEST_EXTERN int main(int argc, char **argv) {
  printf("Running main() from gtest_main.cc\n");
  testing::InitGoogleTest(&argc, argv);

  LibRzuInit();
  GlobalTestConfig::init();

  ConfigInfo::get()->init(argc, argv);

  ConfigInfo::get()->info("rzu version: %s\n", CFG_GET("core.version")->getString().c_str());
  ConfigInfo::get()->info("Auth connection: %s:%d\n", CONFIG_GET()->auth.ip.get().c_str(), CONFIG_GET()->auth.port.get());

  return RUN_ALL_TESTS();
}
