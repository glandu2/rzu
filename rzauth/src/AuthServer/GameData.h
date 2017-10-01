#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "Core/Object.h"
#include <array>
#include <stdint.h>
#include <string>
#include <time.h>
#include <unordered_map>
#include <vector>

class IWritableConsole;

namespace AuthServer {

class GameServerSession;
class ClientData;

class GameData : public Object {
	DECLARE_CLASS(AuthServer::GameData)

public:
	static void init();
	static const std::unordered_map<uint16_t, GameData*>& getServerList() { return servers; }

	static GameData* tryAdd(GameServerSession* gameServerSession,
	                        uint16_t serverIdx,
	                        std::string serverName,
	                        std::string serverIp,
	                        int32_t serverPort,
	                        std::string serverScreenshotUrl,
	                        bool isAdultServer,
	                        const std::array<uint8_t, 16>* guid,
	                        GameData** oldGameData = nullptr);
	static void remove(GameData* gameData);

	void setReady(bool ready) { this->ready = ready; }
	bool isReady() { return getGameServer() != nullptr && ready; }

	void kickClient(ClientData* client);
	void sendNotifyItemPurchased(ClientData* client);

	GameServerSession* getGameServer() { return gameServerSession; }
	void setGameServer(GameServerSession* gameServerSession);

	uint16_t getServerIdx() { return serverIdx; }
	std::string getServerName() { return serverName; }
	std::string getServerIp() { return serverIp; }
	int32_t getServerPort() { return serverPort; }
	std::string getServerScreenshotUrl() { return serverScreenshotUrl; }
	bool getIsAdultServer() { return isAdultServer; }
	const std::array<uint8_t, 16>& getGuid() { return guid; }

	void incPlayerCount() { playerCount++; }
	void decPlayerCount() { playerCount--; }
	uint32_t getPlayerCount() { return playerCount; }
	time_t getCreationTime() { return creationTime; }

protected:
	void updateObjectName();

	static void commandList(IWritableConsole* console, const std::vector<std::string>& args);

private:
	GameData(GameServerSession* gameServerSession,
	         uint16_t serverIdx,
	         std::string serverName,
	         std::string serverIp,
	         int32_t serverPort,
	         std::string serverScreenshotUrl,
	         bool isAdultServer,
	         const std::array<uint8_t, 16>* guid);
	~GameData();

	static std::unordered_map<uint16_t, GameData*> servers;

	GameServerSession* gameServerSession;

	uint16_t serverIdx;
	std::string serverName;
	std::string serverIp;
	int32_t serverPort;
	std::string serverScreenshotUrl;
	bool isAdultServer;
	std::array<uint8_t, 16> guid;

	uint32_t playerCount;
	time_t creationTime;

	bool ready;
};

}  // namespace AuthServer

#endif  // GAMEDATA_H
