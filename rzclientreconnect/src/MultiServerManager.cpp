#include "MultiServerManager.h"
#include "ConnectionToClient.h"
#include "GlobalConfig.h"

MultiServerManager::MultiServerManager(ConnectionToClient* connectionToClient)
    : connectionToClient(connectionToClient), updateClientFromGameData(connectionToClient) {}

MultiServerManager::~MultiServerManager() {
	log(LL_Info, "Client disconnected, closing server connections\n");
	activateConnectionToServer(nullptr);

	for(auto& connection : connections)
		connection.abortSession();
}

void MultiServerManager::onPacketFromClient(const TS_MESSAGE* packet) {
	if(activeMainServer)
		controlledServer->onClientPacketReceived(packet);
}

void MultiServerManager::onPacketFromServer(ConnectionToServer* originatingServer, const TS_MESSAGE* packet) {
	if(activeMainServer == originatingServer) {
		connectionToClient->onPacketFromServer(packet);
	}
}

bool MultiServerManager::isKnownLocalPlayer(std::string_view playerName) {
	for(auto& connection : connections) {
		if(connection.getPlayerName() == playerName)
			return true;
	}
	return false;
}

void MultiServerManager::ensureOneConnectionToServer() {
	if(connections.empty())
		spawnConnectionToServer(CONFIG_GET()->server.account.get(),
		                        CONFIG_GET()->server.password.get(),
		                        CONFIG_GET()->server.playerName.get());
}

void MultiServerManager::onServerDisconnected(ConnectionToServer* disconnectedServer, GameData&& gameData) {
	if(disconnectedServer == activeMainServer) {
		connectionToClient->sendCharMessage("%s disconnected, reconnecting",
		                                    disconnectedServer->getPlayerName().c_str());

		// Remove the player and all inventory items
		updateClientFromGameData.removePlayerData(gameData);
	}
}

void MultiServerManager::activateConnectionToServer(ConnectionToServer* connectionToServer) {
	if(this->activeMainServer)
		updateClientFromGameData.removePlayerData(this->activeMainServer->getGameData());

	this->activeMainServer = connectionToServer;
	this->controlledServer = connectionToServer;

	if(this->activeMainServer)
		updateClientFromGameData.addPlayerData(this->activeMainServer->getGameData());
}

void MultiServerManager::spawnConnectionToServer(const std::string& account,
                                                 const std::string& password,
                                                 const std::string& playername) {
	log(LL_Info, "Spawning server connection %d\n", (int) connections.size());

	connections.emplace_back(this, account, password, playername);
	ConnectionToServer* connection = &connections.back();

	if(!activeMainServer)
		activateConnectionToServer(connection);

	if(CONFIG_GET()->trafficDump.enableServer.get())
		connection->setPacketLogger(this->connectionToClient->getStream()->getPacketLogger());
	else
		connection->setPacketLogger(nullptr);
	connection->connect();
}

ConnectionToServer* MultiServerManager::getConnectionToServerByName(std::string_view playerName) {
	for(auto& connection : connections) {
		if(connection.getPlayerName() == playerName) {
			return &connection;
		}
	}

	return nullptr;
}

ConnectionToServer* MultiServerManager::getConnectionToServerByHandle(ar_handle_t playerHandle) {
	for(auto& connection : connections) {
		if(connection.getLocalPlayerServerHandle() == playerHandle) {
			return &connection;
		}
	}

	return nullptr;
}

bool MultiServerManager::remoteConnectionToServerByName(std::string_view name) {
	std::list<ConnectionToServer>::iterator connectionIt = connections.end();
	for(auto it = connections.begin(); it != connections.end(); ++it) {
		if(it->getPlayerName() == name) {
			connectionIt = it;
			break;
		}
	}
	if(connectionIt != connections.end()) {
		ConnectionToServer* newConnection = &(*connectionIt);
		if(newConnection == activeMainServer)
			activateConnectionToServer(nullptr);
		newConnection->closeSession();
		connections.erase(connectionIt);

		return true;
	} else {
		connectionToClient->sendCharMessage("Error: no connection with player %s", name.data());
		return false;
	}
}

void MultiServerManager::setActiveConnectionToServer(ConnectionToServer* newConnection) {
	if(newConnection) {
		if(newConnection == activeMainServer)
			connectionToClient->sendCharMessage("Warning: player %s was already active player",
			                                    activeMainServer->getPlayerName().c_str());
		connectionToClient->sendCharMessage("Switching to player %s", newConnection->getPlayerName().c_str());
		activateConnectionToServer(newConnection);
	}
}

void MultiServerManager::setControlConnectionToServer(ConnectionToServer* newConnection) {
	if(newConnection) {
		if(newConnection == controlledServer)
			connectionToClient->sendCharMessage("Warning: player %s was already active player",
			                                    activeMainServer->getPlayerName().c_str());
		connectionToClient->sendCharMessage("Switching to player %s", newConnection->getPlayerName().c_str());
		activateConnectionToServer(newConnection);
	}
}
