#include "Terminator.h"

void Terminator::start(std::string host, uint16_t port, std::string command) {
	this->host = host;
	this->port = port;
	this->command = command + "\n";
	connect(host.c_str(), port);
}

EventChain<SocketSession> Terminator::onConnected()
{
	log(LL_Debug, "Connected to %s:%d\n", host.c_str(), port);
	log(LL_Debug, "Sending %s", command.c_str());
	write(command.c_str(), command.size());

	return SocketSession::onConnected();
}

EventChain<SocketSession> Terminator::onDisconnected(bool causedByRemote)
{
	log(LL_Debug, "Disconnected\n");
	return SocketSession::onDisconnected(causedByRemote);
}

EventChain<SocketSession> Terminator::onDataReceived()
{
	if(getStream()) {
		getStream()->discardAll();
	}

	return SocketSession::onDataReceived();
}
