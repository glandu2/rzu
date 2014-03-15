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

	RappelzLibInit(argc, argv, &init);


	PlayerCountMonitor playerCount(CFG("ip", "127.0.0.1").get(), CFG("port", 4500).get());
	playerCount.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

static void init() {
	CFG("global.version", RappelzPlayerCountVersion);
	CFG("ip", "127.0.0.1");
	CFG("port", 4500);
}
