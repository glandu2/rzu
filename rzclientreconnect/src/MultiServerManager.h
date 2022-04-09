#pragma once

#include "ConnectionToServer.h"
#include "IPacketInterface.h"
#include "UpdateClientFromGameData.h"
#include <list>
#include <string_view>

struct TS_MESSAGE;
class ConnectionToClient;

class MultiServerManager : public Object {
	DECLARE_CLASSNAME(MultiServerManager, 0)

public:
	MultiServerManager(ConnectionToClient* connectionToClient);
	~MultiServerManager();

	void onPacketFromClient(const TS_MESSAGE* packet);
	void onPacketFromServer(ConnectionToServer* originatingServer, const TS_MESSAGE* packet);

	const std::list<ConnectionToServer>& getConnections() { return connections; }
	ConnectionToServer* getConnectionToServer() { return activeMainServer; }
	ConnectionToServer* getControlledServer() { return controlledServer; }

	bool isKnownLocalPlayer(std::string_view playerName);

	void ensureOneConnectionToServer();

	void onServerDisconnected(ConnectionToServer*, GameData&& gameData);
	void activateConnectionToServer(ConnectionToServer* connectionToServer);
	void spawnConnectionToServer(const std::string& account,
	                             const std::string& password,
	                             const std::string& playername);

	ConnectionToServer* getConnectionToServerByName(std::string_view playerName);
	ConnectionToServer* getConnectionToServerByHandle(ar_handle_t playerHandle);
	bool remoteConnectionToServerByName(std::string_view playerName);
	void setActiveConnectionToServer(ConnectionToServer* newConnection);
	void setControlConnectionToServer(ConnectionToServer* newConnection);

private:
	ConnectionToClient* connectionToClient;
	UpdateClientFromGameData updateClientFromGameData;

	// Current main player
	ConnectionToServer* activeMainServer = nullptr;
	// The player currently controlled
	ConnectionToServer* controlledServer = nullptr;

	// All connected players
	std::list<ConnectionToServer> connections;
};