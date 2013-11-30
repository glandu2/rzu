#include "Network/SocketPoll.h"
#include "Network/Socket.h"
#include <string.h>

void onData(void* instance, ISocket* sock) {
	char buffer[128] = {0};
	int read;

	while((read = sock->read(buffer, 4096)) > 0) {
		printf("%s", buffer);
		sock->write(buffer, read);
		memset(buffer, 0, 128);
	}
}

void onConnection(void* instance, ISocket* sock) {
	ISocket *newSocket;

	while((newSocket = sock->accept())) {
		printf("new socket\n");
		newSocket->addDataListener(nullptr, &onData);
	}
}

int main() {
	SocketPoll socketPoll;
	Socket socket;

	socket.addConnectionListener(nullptr, &onConnection);
	socket.listen("127.0.0.1", 1234);

	socketPoll.run();
}
