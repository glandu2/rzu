#include "PlayerCountMonitor.h"
#include "uv.h"
#include <string.h>
#include "EventLoop.h"

int main(int argc, char *argv[])
{
	const char* host = "127.0.0.1";
	uint16_t port = 4500;

	if(argc >= 2)
		host = argv[1];

	if(argc >= 3)
		port = atoi(argv[2]);


	PlayerCountMonitor playerCount(host, port);
	playerCount.start();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}
