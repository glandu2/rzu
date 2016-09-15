#include "Terminator.h"
#include <string.h>
#include "LibRzuInit.h"
#include "Config/ConfigInfo.h"
#include "Core/EventLoop.h"
#include "Config/GlobalCoreConfig.h"

static void init();

int main(int argc, char *argv[])
{
	LibRzuInit();
	init();
	ConfigInfo::get()->init(argc, argv);

	Terminator terminator;
	terminator.start(CFG_GET("ip")->getString(), CFG_GET("port")->getInt(), CFG_GET("command")->getString());

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

static void init() {
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4501);
	CFG_CREATE("command", "terminate");
}
