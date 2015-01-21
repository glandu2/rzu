#include <stdio.h>
#include "uv.h"
#include "ClientAuthSession.h"
#include <string.h>
#include "EventLoop.h"
#include "RappelzLibInit.h"
#include "PacketSession.h"
#include "ConfigInfo.h"
#include "RappelzLibConfig.h"
#include "Log.h"
#include "IrcClient.h"
#include "GameSession.h"
#include "ChatAuthSession.h"

#include <vector>
#include <string>
#include <unordered_map>

struct TrafficDump {
	cval<bool> &enable;
	cval<std::string> &dir, &file, &level, &consoleLevel;

	TrafficDump() :
		enable(CFG_CREATE("trafficdump.enable", false)),
		dir(CFG_CREATE("trafficdump.dir", "traffic_log")),
		file(CFG_CREATE("trafficdump.file", "chatgateway.log")),
		level(CFG_CREATE("trafficdump.level", "debug")),
		consoleLevel(CFG_CREATE("trafficdump.consolelevel", "fatal"))
	{
		Utils::autoSetAbsoluteDir(dir);
	}
}* trafficDump;

static void init() {
	CFG_CREATE("rappelz.ip", "127.0.0.1");
	CFG_CREATE("rappelz.port", 4500);

	CFG_CREATE("rappelz.account", "test1");
	CFG_CREATE("rappelz.password", "admin");
	CFG_CREATE("rappelz.gsindex", 1);
	CFG_CREATE("rappelz.playername", "Player");

	CFG_CREATE("irc.ip", "127.0.0.1");
	CFG_CREATE("irc.host", "*");
	CFG_CREATE("irc.port", 6667);
	CFG_CREATE("irc.channel", "");
	CFG_CREATE("irc.nick", "");

	CFG_CREATE("use_rsa", true);
	CFG_CREATE("gateway", false);
	CFG_CREATE("autoreco", 280);
	CFG_CREATE("recodelay", 5000);

	trafficDump = new TrafficDump;

	RappelzLibConfig::get()->app.appName.setDefault("RappelzChatGateway");
	RappelzLibConfig::get()->app.configfile.setDefault("chatgateway.opt");
	RappelzLibConfig::get()->log.file.setDefault("chatgateway.log");
}


int main(int argc, char *argv[])
{
	RappelzLibInit();
	init();
	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(RappelzLibConfig::get()->log.enable,
				   RappelzLibConfig::get()->log.level,
				   RappelzLibConfig::get()->log.consoleLevel,
				   RappelzLibConfig::get()->log.dir,
				   RappelzLibConfig::get()->log.file,
				   RappelzLibConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	Log trafficLogger(trafficDump->enable,
					  trafficDump->level,
					  trafficDump->consoleLevel,
					  trafficDump->dir,
					  trafficDump->file,
					  RappelzLibConfig::get()->log.maxQueueSize);
	Log::setDefaultPacketLogger(&trafficLogger);

	ConfigInfo::get()->dump();

	bool usersa = CFG_GET("use_rsa")->getBool();
	std::string ip = CFG_GET("rappelz.ip")->getString();
	int port = CFG_GET("rappelz.port")->getInt();
	bool enableGateway = CFG_GET("gateway")->getBool();
	int autoReco = CFG_GET("autoreco")->getInt();
	int recoDelay = CFG_GET("recodelay")->getInt();

	std::string ircIp = CFG_GET("irc.ip")->getString();
	std::string ircHost = CFG_GET("irc.host")->getString();
	int ircPort = CFG_GET("irc.port")->getInt();
	std::string ircNick = CFG_GET("irc.nick")->getString();
	std::string ircChannel = CFG_GET("irc.channel")->getString();

	std::string account = CFG_GET("rappelz.account")->getString();
	std::string password = CFG_GET("rappelz.password")->getString();
	int serverIdx = CFG_GET("rappelz.gsindex")->getInt();
	std::string playername = CFG_GET("rappelz.playername")->getString();

	mainLogger.info("Starting chat gateway\n");

	GameSession* gameSession = new GameSession(playername, enableGateway, &trafficLogger);
	IrcClient* ircClient = new IrcClient(ircIp, ircPort, ircHost, ircChannel, ircNick, &trafficLogger);

	ChatAuthSession* authSession = new ChatAuthSession(gameSession, ip, port, account, password, serverIdx, recoDelay, autoReco, usersa ? ClientAuthSession::ACM_RSA_AES : ClientAuthSession::ACM_DES);

	gameSession->setIrcClient(ircClient);
	ircClient->setGameSession(gameSession);

	authSession->connect();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}
