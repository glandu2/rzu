#include "PlayerCountMonitor.h"
#include "uv.h"
#include <string.h>
#include "RappelzLibInit.h"
#include "ConfigInfo.h"
#include "RappelzPlayerCountGitVersion.h"

static void init();

int main(int argc, char *argv[])
{
	const char* host = "127.0.0.1";
	uint16_t port = 4500;

	if(argc >= 2)
		host = argv[1];

	if(argc >= 3)
		port = atoi(argv[2]);

	RappelzLibInit(argc, argv, &init);


	PlayerCountMonitor playerCount(host, port);
	playerCount.start();

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

static void init() {
	CFG("global.version", RappelzPlayerCountVersion);
}
