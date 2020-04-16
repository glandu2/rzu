#include "Terminator.h"
#include "Core/Utils.h"

void Terminator::start(std::string host, uint16_t port, std::string command, int timeoutMs) {
	this->host = host;
	this->port = port;
	this->command = command + "\n";

	Utils::stringReplaceAll(this->command, "\\n", "\n");

	timeoutTimer.start(this, &Terminator::onTimeout, timeoutMs, 0);
	connect(host.c_str(), port);
}

EventChain<SocketSession> Terminator::onConnected() {
	log(LL_Debug, "Connected to %s:%d\n", host.c_str(), port);
	log(LL_Debug, "Sending %s", command.c_str());
	write(command.c_str(), command.size());

	return SocketSession::onConnected();
}

EventChain<SocketSession> Terminator::onDisconnected(bool causedByRemote) {
	log(LL_Debug, "Disconnected\n");
	timeoutTimer.stop();
	return SocketSession::onDisconnected(causedByRemote);
}

EventChain<SocketSession> Terminator::onDataReceived() {
	if(getStream()) {
		std::vector<char> dataRecv;
		getStream()->readAll(&dataRecv);
		log(LL_Debug, "Received data: %*s\n", (int) dataRecv.size(), dataRecv.data());
	}

	return SocketSession::onDataReceived();
}

void Terminator::onTimeout() {
	closeSession();
}
