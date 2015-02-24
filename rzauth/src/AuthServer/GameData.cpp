#include "GameData.h"
#include "GameServerSession.h"
#include "ClientData.h"
#include "LogServerClient.h"

namespace AuthServer {

std::unordered_map<uint16_t, GameData*> GameData::servers;

GameData::GameData(GameServerSession *gameServerSession,
				   uint16_t serverIdx,
				   std::string serverName,
				   std::string serverIp,
				   int32_t serverPort,
				   std::string serverScreenshotUrl,
				   bool isAdultServer)
	: gameServerSession(gameServerSession),
	  serverIdx(serverIdx),
	  serverName(serverName),
	  serverIp(serverIp),
	  serverPort(serverPort),
	  serverScreenshotUrl(serverScreenshotUrl),
	  isAdultServer(isAdultServer),
	  playerCount(0),
	  ready(false)
{
	setDirtyObjectName();

	LogServerClient::sendLog(LogServerClient::LM_GAME_SERVER_LOGIN, 0, 0, 0, serverIdx, serverPort, 0, 0, 0, 0, 0, 0,
							 0, 0, 0, 0, serverIp.c_str(), -1, serverName.c_str(), -1);
}

GameData::~GameData() {
	LogServerClient::sendLog(LogServerClient::LM_GAME_SERVER_LOGOUT, 0, 0, 0, serverIdx, serverPort, 0, 0, 0, 0, 0, 0,
							 0, 0, 0, 0, serverIp.c_str(), -1, serverName.c_str(), -1);
	ClientData::removeServer(this);
}

GameData* GameData::tryAdd(GameServerSession* gameServerSession,
						uint16_t serverIdx,
						std::string serverName,
						std::string serverIp,
						int32_t serverPort,
						std::string serverScreenshotUrl,
						bool isAdultServer,
						GameData** oldGameData)
{
	auto it = servers.find(serverIdx);

	if(oldGameData)
		*oldGameData = nullptr;

	if(it == servers.end()) {
		GameData* gameData = new GameData(gameServerSession,
										  serverIdx,
										  serverName,
										  serverIp,
										  serverPort,
										  serverScreenshotUrl,
										  isAdultServer);

		servers.insert(std::pair<uint16_t, GameData*>(serverIdx, gameData));

		return gameData;
	} else {
		if(oldGameData)
			*oldGameData = it->second;

		return nullptr;
	}
}

void GameData::remove(GameData* gameData) {
	servers.erase(gameData->serverIdx);
	delete gameData;
}

void GameData::setGameServer(GameServerSession *gameServerSession) {
	GameServerSession* oldGameServerSession = this->gameServerSession;

	if(this->gameServerSession != gameServerSession) {
		this->gameServerSession = gameServerSession;

		if(oldGameServerSession)
			oldGameServerSession->setGameData(nullptr);
		if(gameServerSession)
			gameServerSession->setGameData(this);
	}
}

void GameData::kickClient(ClientData *client) {
	if(gameServerSession) {
		gameServerSession->kickClient(client);
	} else {
		info("Game server %s not reachable, not kicking account %s\n", serverName.c_str(), client->account.c_str());
	}
}

void GameData::sendNotifyItemPurchased(ClientData* client) {
	if(gameServerSession) {
		gameServerSession->sendNotifyItemPurchased(client);
	} else {
		info("Game server %s not reachable, not notifying account %s\n", serverName.c_str(), client->account.c_str());
	}
}

void GameData::updateObjectName() {
	setObjectName(10 + (int)serverName.size(), "GameData[%s]", serverName.c_str());
}

} // namespace AuthServer
