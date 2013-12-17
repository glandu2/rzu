#include "ClientInfo.h"
#include "ServerInfo.h"
#include "EventLoop.h"
#include "ConfigInfo.h"

//socket->deleteLater in uv_check_t
void showDebug(uv_timer_t*, int);

int main() {
//	uv_timer_t timer;
//	uv_timer_init(EventLoop::getLoop(), &timer);
//	uv_timer_start(&timer, &showDebug, 0, 3000);

	ConfigInfo::get()->readFile("Auth.opt");
	ConfigInfo::get()->dump(stdout);

	ClientInfo::startServer();
	ServerInfo::startServer();

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

class Server;
class CircularBuffer;
class DB_Account;


void showDebug(uv_timer_t *, int) {
	char debugInfo[1000];
	strcpy(debugInfo, "----------------------------------\n");
	sprintf(debugInfo, "%s%lu Object\n", debugInfo, ClassCounter<Object>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu EventLoop\n", debugInfo, ClassCounter<EventLoop>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu Socket\n", debugInfo, ClassCounter<Socket>::getObjectCount());
	sprintf(debugInfo, "%s\t\t%lu EncryptedSocket\n", debugInfo, ClassCounter<EncryptedSocket>::getObjectCount());
	sprintf(debugInfo, "%s\t\t\t%lu RappelzSocket\n", debugInfo, ClassCounter<RappelzSocket>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu CircularBuffer\n", debugInfo, ClassCounter<CircularBuffer>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu RC4Cipher\n", debugInfo, ClassCounter<RC4Cipher>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu Server\n", debugInfo, ClassCounter<Server>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ClientInfo\n", debugInfo, ClassCounter<ClientInfo>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ClientData\n", debugInfo, ClassCounter<ClientData>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu DB_Account\n", debugInfo, ClassCounter<DB_Account>::getObjectCount());
	sprintf(debugInfo, "%s\t%lu ServerInfo\n", debugInfo, ClassCounter<ServerInfo>::getObjectCount());
	printf(debugInfo);
}
