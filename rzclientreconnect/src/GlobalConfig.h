#pragma once

#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/Utils.h"
#include "Packet/PacketEpics.h"

struct GlobalConfig {
	struct GeneralConfig {
		cval<int>& epic;
		cval<int>& delayTime;
		cval<int>& recoDelay;

		GeneralConfig()
		    : epic(CFG_CREATE("general.epic", EPIC_LATEST)),
		      delayTime(CFG_CREATE("general.delayTime", 5000)),
		      recoDelay(CFG_CREATE("general.recoDelay", 280)) {}
	} generalConfig;

	struct AuthConfig {
		cval<std::string>& ip;
		cval<int>& port;
		cval<std::string>& account;
		cval<std::string>& password;
		cval<int>& serverIdx;
		cval<std::string>& playerName;

		AuthConfig()
		    : ip(CFG_CREATE("server.ip", "remote.auth.server.ip")),  // user must change this
		      port(CFG_CREATE("server.port", 4500)),
		      account(CFG_CREATE("server.account", "")),
		      password(CFG_CREATE("server.password", "", true)),
		      serverIdx(CFG_CREATE("server.gsindex", 0)),
		      playerName(CFG_CREATE("server.playername", "")) {}
	} server;

	struct GameConfig : IListener {
		ListenerConfig listener;
		cval<std::string>& gameExternalIp;
		cval<int>& gamePort;

		GameConfig()
		    : listener("client.listen", "127.0.0.1", 4500, true, 0),
		      gameExternalIp(CFG_CREATE("client.externalip", "127.0.0.1")),
		      gamePort(CFG_CREATE("client.gsport", 34514)) {
			listener.listenIp.addListener(this, &updateDefaultExternalIp);
		}

		static void updateDefaultExternalIp(IListener* instance);
	} client;

	struct TrafficDump {
		cval<bool>& enable;
		cval<bool>& enableServer;
		cval<std::string>&dir, &file, &level, &consoleLevel;

		TrafficDump()
		    : enable(CFG_CREATE("trafficdump.enable", false)),
		      enableServer(CFG_CREATE("trafficdump.enable_server", false)),
		      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		      file(CFG_CREATE("trafficdump.file", "rzclientreconnect.log")),
		      level(CFG_CREATE("trafficdump.level", "debug")),
		      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal")) {
			Utils::autoSetAbsoluteDir(dir);
		}
	} trafficDump;

	static GlobalConfig* get();
	static void init();
};

#define CONFIG_GET() GlobalConfig::get()

