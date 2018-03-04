#include "GameClientSession.h"

GameClientSession::GameClientSession(Parameter* serverAddress)
    : ClientSession(false), serverIp(serverAddress->serverIp), serverPort(serverAddress->serverPort) {}

GameClientSession::~GameClientSession() {}

std::string GameClientSession::getServerIp() {
	return serverIp;
}

uint16_t GameClientSession::getServerPort() {
	return serverPort;
}