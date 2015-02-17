#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "Object.h"
#include <stdint.h>
#include <unordered_map>
#include <string>

namespace AuthServer {

class GameServerSession;
class ClientData;

class GameData : public Object
{
public:
	//multithread concurrency when GS is disconnected while new players connect to it
	static const std::unordered_map<uint16_t, GameData*>& getServerList() { return servers; }
	static GameData* tryAdd(GameServerSession* gameServerSession,
							uint16_t serverIdx,
							std::string serverName,
							std::string serverIp,
							int32_t serverPort,
							std::string serverScreenshotUrl,
							bool isAdultServer,
							GameData **oldGameData = nullptr);
	static void remove(GameData* gameData);

	void kickClient(ClientData *client);
	void sendNotifyItemPurchased(ClientData* client);

	GameServerSession* getGameServer() { return gameServerSession; }
	bool setGameServer(GameServerSession* gameServerSession);

	uint16_t getServerIdx() { return serverIdx; }
	std::string getServerName() { return serverName; }
	std::string getServerIp() { return serverIp; }
	int32_t getServerPort() { return serverPort; }
	std::string getServerScreenshotUrl() { return serverScreenshotUrl; }
	bool getIsAdultServer() { return isAdultServer; }

	void incPlayerCount() { playerCount++; }
	void decPlayerCount() { playerCount--; }
	uint32_t getPlayerCount() { return playerCount; }

protected:
	void updateObjectName();

private:
	GameData(GameServerSession* gameServerSession,
			 uint16_t serverIdx,
			 std::string serverName,
			 std::string serverIp,
			 int32_t serverPort,
			 std::string serverScreenshotUrl,
			 bool isAdultServer);
	~GameData();

	static std::unordered_map<uint16_t, GameData*> servers;

	uint16_t serverIdx;
	std::string serverName;
	std::string serverIp;
	int32_t serverPort;
	std::string serverScreenshotUrl;
	bool isAdultServer;
	uint32_t playerCount;
	GameServerSession* gameServerSession;
};

} // namespace AuthServer

#endif // GAMEDATA_H
