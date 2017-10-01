#include "GlobalConfig.h"
#include "Config/GlobalCoreConfig.h"

namespace AuthServer {

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();
}

}  // namespace AuthServer
