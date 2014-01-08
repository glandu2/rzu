#ifndef UPLOADREQUEST_H
#define UPLOADREQUEST_H

#include <unordered_map>
#include <stdint.h>
#include "uv.h"
#include <time.h>

namespace UploadServer {

class GameServerSession;

class UploadRequest
{
public:
	UploadRequest(GameServerSession *gameServer, uint32_t client_id, uint32_t account_id, uint32_t guild_sid, uint32_t one_time_password);

	GameServerSession* getGameServer() { return gameServer; }
	uint32_t getClientId() { return client_id; }
	uint32_t getAccountId() { return account_id; }
	uint32_t getGuildId() { return guild_sid; }
	uint32_t getOneTimePassword() { return one_time_password; }

	static UploadRequest* pushRequest(GameServerSession *gameServer, uint32_t client_id, uint32_t account_id, uint32_t guild_sid, uint32_t one_time_password);
	static UploadRequest* popRequest(uint32_t client_id, uint32_t account_id, uint32_t guild_sid, uint32_t one_time_password, const std::string& gameServerName);
	static unsigned int getClientCount() { return pendingRequests.size(); }

private:
	static uv_mutex_t initializeLock();

	static std::unordered_map<uint32_t, UploadRequest*> pendingRequests;
	static uv_mutex_t mapLock;
	static uv_once_t lockInit;

	GameServerSession *gameServer;
	uint32_t client_id;
	uint32_t account_id;
	uint32_t guild_sid;
	uint32_t one_time_password;
	time_t timestamp;
};

}

#endif // UPLOADREQUEST_H
