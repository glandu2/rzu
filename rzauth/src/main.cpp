#include "Network/SocketPoll.h"
#include "ClientInfo.h"

int main() {
	SocketPoll socketPoll;

	ClientInfo::startServer();

	socketPoll.run();
}
