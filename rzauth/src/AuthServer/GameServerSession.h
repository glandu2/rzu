#ifndef GAMESERVERSESSION_H
#define GAMESERVERSESSION_H

#include "RappelzSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"

#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_KICK_FAILED.h"

namespace AuthServer {

class GameServerSession : public RappelzSession
{
	DECLARE_CLASS(AuthServer::GameServerSession)

public:
	GameServerSession();

	//multithread concurrency when GS is disconnected while new players connect to it
	static const std::unordered_map<uint16_t, GameServerSession*>& getServerList() { return servers; }

	uint16_t getServerIdx() { return serverIdx; }
	std::string getServerName() { return serverName; }
	std::string getServerIp() { return serverIp; }
	int32_t getServerPort() { return serverPort; }
	std::string getServerScreenshotUrl() { return serverScreenshotUrl; }
	bool getIsAdultServer() { return isAdultServer; }

	void kickClient(const std::string& account);

	void incPlayerCount() { playerCount++; }
	void decPlayerCount() { playerCount--; }
	uint32_t getPlayerCount() { return playerCount; }

protected:
	void onPacketReceived(const TS_MESSAGE* packet);

	void onServerLogin(const TS_GA_LOGIN* packet);
	void onClientLogin(const TS_GA_CLIENT_LOGIN* packet);
	void onClientLogout(const TS_GA_CLIENT_LOGOUT* packet);
	void onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet);

private:
	~GameServerSession();

	static std::unordered_map<uint16_t, GameServerSession*> servers;

	uint16_t serverIdx;
	std::string serverName;
	std::string serverIp;
	int32_t serverPort;
	std::string serverScreenshotUrl;
	bool isAdultServer;
	uint32_t playerCount;
};

} // namespace AuthServer

#endif // GAMESERVERSESSION_H
