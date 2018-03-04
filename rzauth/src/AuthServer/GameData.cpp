#include "GameData.h"
#include "ClientData.h"
#include "Console/ConsoleCommands.h"
#include "Core/Utils.h"
#include "GameServerSession.h"
#include "LogServerClient.h"

namespace AuthServer {

std::unordered_map<uint16_t, GameData*> GameData::servers;

void GameData::init() {
	ConsoleCommands::get()->addCommand(
	    "gameserver.list", "list", 0, 0, &commandList, "List all game servers", "list : list all game servers");
}

GameData::GameData(GameServerSession* gameServerSession,
                   uint16_t serverIdx,
                   std::string serverName,
                   std::string serverIp,
                   int32_t serverPort,
                   std::string serverScreenshotUrl,
                   bool isAdultServer,
                   const std::array<uint8_t, 16>* guid)
    : gameServerSession(gameServerSession),
      serverIdx(serverIdx),
      serverName(serverName),
      serverIp(serverIp),
      serverPort(serverPort),
      serverScreenshotUrl(serverScreenshotUrl),
      isAdultServer(isAdultServer),
      playerCount(0),
      creationTime(time(nullptr)),
      ready(false) {
	setDirtyObjectName();

	if(guid != nullptr) {
		this->guid = *guid;
	} else {
		this->guid = std::array<uint8_t, 16>{};
	}

	LogServerClient::sendLog(LogServerClient::LM_GAME_SERVER_LOGIN,
	                         0,
	                         0,
	                         0,
	                         serverIdx,
	                         serverPort,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         serverIp.c_str(),
	                         -1,
	                         serverName.c_str(),
	                         -1);
}

GameData::~GameData() {
	LogServerClient::sendLog(LogServerClient::LM_GAME_SERVER_LOGOUT,
	                         0,
	                         0,
	                         0,
	                         serverIdx,
	                         serverPort,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         serverIp.c_str(),
	                         -1,
	                         serverName.c_str(),
	                         -1);
	ClientData::removeServer(this);
}

GameData* GameData::tryAdd(GameServerSession* gameServerSession,
                           uint16_t serverIdx,
                           std::string serverName,
                           std::string serverIp,
                           int32_t serverPort,
                           std::string serverScreenshotUrl,
                           bool isAdultServer,
                           const std::array<uint8_t, 16>* guid,
                           GameData** oldGameData) {
	auto it = servers.find(serverIdx);

	if(oldGameData)
		*oldGameData = nullptr;

	if(it == servers.end()) {
		GameData* gameData = new GameData(
		    gameServerSession, serverIdx, serverName, serverIp, serverPort, serverScreenshotUrl, isAdultServer, guid);

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

void GameData::setGameServer(GameServerSession* gameServerSession) {
	GameServerSession* oldGameServerSession = this->gameServerSession;

	if(this->gameServerSession != gameServerSession) {
		this->gameServerSession = gameServerSession;

		if(oldGameServerSession)
			oldGameServerSession->setGameData(nullptr);
		if(gameServerSession)
			gameServerSession->setGameData(this);
	}
}

void GameData::kickClient(ClientData* client) {
	if(gameServerSession) {
		gameServerSession->kickClient(client);
	} else {
		log(LL_Info,
		    "Game server %s not reachable, not kicking account %s\n",
		    serverName.c_str(),
		    client->account.c_str());
	}
}

void GameData::sendNotifyItemPurchased(ClientData* client) {
	if(gameServerSession) {
		gameServerSession->sendNotifyItemPurchased(client);
	} else {
		log(LL_Info,
		    "Game server %s not reachable, not notifying account %s\n",
		    serverName.c_str(),
		    client->account.c_str());
	}
}

void GameData::updateObjectName() {
	setObjectName(10 + (int) serverName.size(), "GameData[%s]", serverName.c_str());
}

void GameData::commandList(IWritableConsole* console, const std::vector<std::string>& args) {
	const std::unordered_map<uint16_t, AuthServer::GameData*>& serverList = AuthServer::GameData::getServerList();
	std::unordered_map<uint16_t, AuthServer::GameData*>::const_iterator it, itEnd;

	for(it = serverList.cbegin(), itEnd = serverList.cend(); it != itEnd; ++it) {
		AuthServer::GameData* server = it->second;

		struct tm upTime;
		Utils::getGmTime(time(nullptr) - server->getCreationTime(), &upTime);

		console->writef(
		    "index: %d, name: %s, address: %s:%d, players count: %u, uptime: %d:%02d:%02d:%02d, screenshot url: %s\r\n",
		    server->getServerIdx(),
		    server->getServerName().c_str(),
		    server->getServerIp().c_str(),
		    server->getServerPort(),
		    server->getPlayerCount(),
		    (upTime.tm_year - 1970) * 365 + upTime.tm_yday,
		    upTime.tm_hour,
		    upTime.tm_min,
		    upTime.tm_sec,
		    server->getServerScreenshotUrl().c_str());
	}
}

}  // namespace AuthServer
