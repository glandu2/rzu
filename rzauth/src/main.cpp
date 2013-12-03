#include "Network/SocketPoll.h"
#include "ClientInfo.h"
#include "ServerInfo.h"

int main() {
	SocketPoll socketPoll;

	ClientInfo::startServer();
	ServerInfo::startServer();

	socketPoll.run();
}
