#include "AutoAuthSession.h"
#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "GameSession.h"
#include "LibRzuInit.h"
#include "uv.h"
#include <stdio.h>
#include <string.h>

#include <string>
#include <unordered_map>
#include <vector>

struct TrafficDump {
	cval<bool>& enable;
	cval<std::string>&dir, &file, &level, &consoleLevel;

	TrafficDump()
	    : enable(CFG_CREATE("trafficdump.enable", false)),
	      dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
	      file(CFG_CREATE("trafficdump.file", "chatgateway.log")),
	      level(CFG_CREATE("trafficdump.level", "debug")),
	      consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal")) {
		Utils::autoSetAbsoluteDir(dir);
	}
} * trafficDump;

static void init() {
	CFG_CREATE("game.ip", "127.0.0.1");
	CFG_CREATE("game.port", 4500);

	CFG_CREATE("game.account", "test1");
	CFG_CREATE("game.password", "admin");
	CFG_CREATE("game.gsindex", 1);
	CFG_CREATE("game.playername", "Player");

	CFG_CREATE("use_rsa", true);
	CFG_CREATE("autoreco", 0);
	CFG_CREATE("recodelay", 5000);

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

	bool usersa = CFG_GET("use_rsa")->getBool();
	std::string ip = CFG_GET("game.ip")->getString();
	int port = CFG_GET("game.port")->getInt();
	int autoReco = CFG_GET("autoreco")->getInt();
	int recoDelay = CFG_GET("recodelay")->getInt();

	std::string account = CFG_GET("game.account")->getString();
	std::string password = CFG_GET("game.password")->getString();
	int serverIdx = CFG_GET("game.gsindex")->getInt();
	std::string playername = CFG_GET("game.playername")->getString();

	Object::logStatic(Object::LL_Info, "main", "Starting server time monitor\n");

	GameSession* gameSession = new GameSession(playername, &trafficLogger);
	AutoAuthSession* authSession =
	    new AutoAuthSession(gameSession,
	                        ip,
	                        port,
	                        account,
	                        password,
	                        serverIdx,
	                        recoDelay,
	                        autoReco,
	                        usersa ? ClientAuthSession::ACM_RSA_AES : ClientAuthSession::ACM_DES);

	authSession->connect();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	return 0;
}
