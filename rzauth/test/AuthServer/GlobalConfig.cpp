#include "GlobalConfig.h"
#include "Config/GlobalCoreConfig.h"

namespace AuthServer {

GlobalConfig* GlobalConfig::get() {
	static GlobalConfig config;
	return &config;
}

void GlobalConfig::init() {
	GlobalConfig::get();

#ifdef _WIN32
	GlobalConfig::get()->connectionString.setDefault("DRIVER={SQLite3 ODBC Driver};Database=AuthDatabase.db;");
#else
	GlobalConfig::get()->connectionString.setDefault("DRIVER=SQLite3;Database=AuthDatabase.db;");
#endif

}

} // namespace AuthServer
