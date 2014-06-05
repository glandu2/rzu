#include "PlayerCountMonitor.h"
#include "uv.h"
#include <string.h>
#include "RappelzLibInit.h"
#include "ConfigInfo.h"
#include "RappelzPlayerCountGitVersion.h"
#include "EventLoop.h"

static void init();

int main(int argc, char *argv[])
{
	RappelzLibInit();
	init();
	ConfigInfo::get()->init(argc, argv);
	ConfigInfo::get()->dump();


	PlayerCountMonitor playerCount(CFG_GET("ip")->getString(), CFG_GET("port")->getInt());
	playerCount.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

static void init() {
	CFG_CREATE("global.version", RappelzPlayerCountVersion);
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4500);
}
