#include "GameClientSessionManager.h"
#include "Core/Utils.h"

GameClientSessionManager::GameClientSessionManager(FilterManager* filterManager, FilterManager* converterFilterManager)
    : filterManager(filterManager), converterFilterManager(converterFilterManager) {}

GameClientSessionManager::~GameClientSessionManager() {}

uint16_t GameClientSessionManager::ensureListening(std::string listenIp,
                                                   uint16_t listenPort,
                                                   std::string serverIp,
                                                   uint16_t serverPort,
                                                   Log* trafficLogger,
                                                   BanManager* banManager) {
	std::string key = serverIp + ":" + Utils::convertToString(serverPort);
	std::unique_ptr<ServerFilter>& serverFilter = gameClientServers[key];

	if(!serverFilter) {
		serverFilter.reset(new ServerFilter(trafficLogger, banManager));
	}

	if(serverFilter->sessionServer.isStarted()) {
		Stream* serverStream = serverFilter->sessionServer.getServerStream();
		if(serverStream)
			return serverStream->getLocalAddress().port;
	} else {
		serverFilter->serverParameters.serverIp = serverIp;
		serverFilter->serverParameters.serverPort = serverPort;
		serverFilter->serverParameters.filterManager = filterManager;
		serverFilter->serverParameters.converterFilterManager = converterFilterManager;
		serverFilter->listenIp.set(listenIp);
		serverFilter->listenPort.set(listenPort);
		serverFilter->sessionServer.start();
		Stream* serverStream = serverFilter->sessionServer.getServerStream();
		if(serverFilter->sessionServer.isStarted() && serverStream)
			return serverStream->getLocalAddress().port;
	}

	return 0;
}

bool GameClientSessionManager::start() {
	started = true;
	return true;
}

void GameClientSessionManager::stop() {
	started = false;
	auto it = gameClientServers.begin();
	auto itEnd = gameClientServers.end();
	for(; it != itEnd; ++it) {
		ServerFilter* server = it->second.get();
		server->sessionServer.stop();
	}
}

bool GameClientSessionManager::isStarted() {
	return started || !gameClientServers.empty();
}
