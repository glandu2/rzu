#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/Utils.h"
#include "Packet/PacketEpics.h"

struct GlobalConfig {
	struct AuthConfig {
		cval<std::string>& ip;
		cval<int>& port;
		cval<int>& epic;
		cval<bool>& strictforward;

		AuthConfig()
		    : ip(CFG_CREATE("server.ip", "remote.auth.server.ip")),  // user must change this
		      port(CFG_CREATE("server.port", 4500)),
		      epic(CFG_CREATE("server.epic", EPIC_LATEST)),
		      strictforward(CFG_CREATE("server.strictforward", false)) {}
	} server;

	struct GameConfig : IListener {
		ListenerConfig listener;
		cval<std::string>& gameExternalIp;
		cval<int>& gameBasePort;
		cval<int>& epic;
		cval<bool>& strictforward;

		GameConfig()
		    : listener("client.listen", "127.0.0.1", 4500, true, 0),
		      gameExternalIp(CFG_CREATE("client.externalip", "127.0.0.1")),
		      gameBasePort(CFG_CREATE("client.gsbaseport", 0)),
		      epic(CFG_CREATE("client.epic", EPIC_LATEST)),
		      strictforward(CFG_CREATE("client.strictforward", false)) {
			listener.listenIp.addListener(this, &updateDefaultExternalIp);
		}

		static void updateDefaultExternalIp(IListener* instance);
	} client;

	struct FilterConfig {
		cval<std::string>& filterModuleName;
		cval<std::string>& converterFilterModuleName;
		cval<bool>& filterModuleEnable;
		cval<bool>& converterFilterModuleEnable;

		FilterConfig()
		    : filterModuleName(CFG_CREATE("filter.modulename", "rzfilter_lua_module")),
		      converterFilterModuleName(CFG_CREATE("filter.convertermodulename", "rzfilter_version_converter")),
		      filterModuleEnable(CFG_CREATE("filter.modulename.enable", true)),
		      converterFilterModuleEnable(CFG_CREATE("filter.convertermodulename.enable", true)) {
			Utils::autoSetAbsoluteDir(filterModuleName);
			Utils::autoSetAbsoluteDir(converterFilterModuleName);
		}
	} filter;

	struct TrafficDump {
		cval<bool>& enable;
		cval<bool>& enableServer;
		cval<std::string>&dir, &file, &level, &consoleLevel;

		TrafficDump()
		    : enable(CFG_CREATE("trafficdump.enable", true)),
		      enableServer(CFG_CREATE("trafficdump.enable_server", false)),
		      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		      file(CFG_CREATE("trafficdump.file", "rzfilter.log")),
		      level(CFG_CREATE("trafficdump.level", "debug")),
		      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "info")) {
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

#endif  // GLOBALCONFIG_H
