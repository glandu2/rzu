#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "GameSession.h"
#include "LibRzuInit.h"
#include "uv.h"
#include <stdio.h>
#include <string.h>

#include "Packet/PacketEpics.h"
#include <string>
#include <unordered_map>
#include <vector>

struct TrafficDump {
	cval<bool>& enable;
	cval<std::string>&dir, &file, &level, &consoleLevel;

	TrafficDump()
	    : enable(CFG_CREATE("trafficdump.enable", false)),
	      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
	      file(CFG_CREATE("trafficdump.file", "rzservertime.log")),
	      level(CFG_CREATE("trafficdump.level", "debug")),
	      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal")) {
		Utils::autoSetAbsoluteDir(dir);
	}
} * trafficDump;

static void init() {
	CFG_CREATE("game.ip", "127.0.0.1");
	CFG_CREATE("game.port", 4500);

	CFG_CREATE("game.account", "test1");
	CFG_CREATE("game.password", "admin", true);
	CFG_CREATE("game.gsindex", 1);
	CFG_CREATE("game.playername", "Player");

	CFG_CREATE("autoreco", 0);
	CFG_CREATE("recodelay", 5000);
	CFG_CREATE("server.epic", EPIC_LATEST);

	trafficDump = new TrafficDump;
}

int main(int argc, char* argv[]) {
	LibRzuScopedUse useLibRzu;
	init();
	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
	               GlobalCoreConfig::get()->log.level,
	               GlobalCoreConfig::get()->log.consoleLevel,
	               GlobalCoreConfig::get()->log.dir,
	               GlobalCoreConfig::get()->log.file,
	               GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	Log trafficLogger(trafficDump->enable,
	                  trafficDump->level,
	                  trafficDump->consoleLevel,
	                  trafficDump->dir,
	                  trafficDump->file,
	                  GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultPacketLogger(&trafficLogger);

	ConfigInfo::get()->dump();

	std::string ip = CFG_GET("game.ip")->getString();
	int port = CFG_GET("game.port")->getInt();
	int autoReco = CFG_GET("autoreco")->getInt();
	int recoDelay = CFG_GET("recodelay")->getInt();

	std::string account = CFG_GET("game.account")->getString();
	std::string password = CFG_GET("game.password")->getString();
	int serverIdx = CFG_GET("game.gsindex")->getInt();
	std::string playername = CFG_GET("game.playername")->getString();
	int epic = CFG_GET("server.epic")->getInt();

	Object::logStatic(Object::LL_Info, "main", "Starting server time monitor on epic 0x%06X\n", epic);

	GameSession gameSession(ip, port, account, password, serverIdx, playername, epic, recoDelay, autoReco);
	gameSession.connect();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	return 0;
}
