#include "GameClientSessionManager.h"
#include "Core/Utils.h"

GameClientSessionManager::GameClientSessionManager()
{

}

GameClientSessionManager::~GameClientSessionManager()
{

}

uint16_t GameClientSessionManager::ensureListening(std::string listenIp, std::string serverIp, uint16_t serverPort)
{
	std::string key = serverIp + ":" + Utils::convertToString(serverPort);
	std::unique_ptr<ServerFilter>& serverFilter = gameClientServers[key];

	if(!serverFilter)
		serverFilter.reset(new ServerFilter);

	if(serverFilter->sessionServer.isStarted()) {
		Stream* serverStream = serverFilter->sessionServer.getServerStream();
		if(serverStream)
			return serverStream->getLocalPort();
	} else {
		serverFilter->serverParameters.serverIp = serverIp;
		serverFilter->serverParameters.serverPort = serverPort;
		serverFilter->listenIp.set(listenIp);
		serverFilter->listenPort.set(0);
		serverFilter->sessionServer.start();
		Stream* serverStream = serverFilter->sessionServer.getServerStream();
		if(serverFilter->sessionServer.isStarted() && serverStream)
			return serverStream->getLocalPort();
	}

	return 0;
}

bool GameClientSessionManager::start()
{
	started = true;
	return true;
}

void GameClientSessionManager::stop()
{
	started = false;
	auto it = gameClientServers.begin();
	auto itEnd = gameClientServers.end();
	for(; it != itEnd; ++it) {
		ServerFilter* server = it->second.get();
		server->sessionServer.stop();
	}
}

bool GameClientSessionManager::isStarted()
{
	return started || !gameClientServers.empty();
}
