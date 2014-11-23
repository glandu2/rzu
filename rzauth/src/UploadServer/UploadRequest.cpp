#include "UploadRequest.h"
#include "uv.h"
#include <time.h>
#include "GameServerSession.h"

namespace UploadServer {

uv_mutex_t UploadRequest::mapLock = initializeLock();
std::unordered_map<uint32_t, UploadRequest*> UploadRequest::pendingRequests;

uv_mutex_t UploadRequest::initializeLock() {
	uv_mutex_init(&mapLock);
	return mapLock;
}

UploadRequest::UploadRequest(GameServerSession *gameServer, uint32_t client_id, uint32_t account_id, uint32_t guild_sid, uint32_t one_time_password) :
	gameServer(gameServer),
	client_id(client_id),
	account_id(account_id),
	guild_sid(guild_sid),
	one_time_password(one_time_password),
	timestamp(time(NULL))
{}

UploadRequest* UploadRequest::pushRequest(GameServerSession *gameServer, uint32_t client_id, uint32_t account_id, uint32_t guild_sid, uint32_t one_time_password) {
	std::pair< std::unordered_map<uint32_t, UploadRequest*>::iterator, bool> result;
	UploadRequest* oldRequest;
	UploadRequest* newRequest = new UploadRequest(gameServer, client_id, account_id, guild_sid, one_time_password);

	uv_mutex_lock(&mapLock);

	result = pendingRequests.insert(std::pair<uint32_t, UploadRequest*>(newRequest->client_id, newRequest));
	if(result.second == false) {
		oldRequest = result.first->second;

		oldRequest->one_time_password = newRequest->one_time_password;
		oldRequest->timestamp = newRequest->timestamp;

		delete newRequest;
		newRequest = oldRequest;
	}

	uv_mutex_unlock(&mapLock);

	return newRequest;
}

UploadRequest* UploadRequest::popRequest(uint32_t client_id, uint32_t account_id, uint32_t guild_sid, uint32_t one_time_password, const std::string& gameServerName) {
	UploadRequest* ret;
	std::unordered_map<uint32_t, UploadRequest*>::iterator it;

	uv_mutex_lock(&mapLock);

	it = pendingRequests.find(client_id);
	if(it != pendingRequests.end()) {
		ret = it->second;
		if(ret->getAccountId() == account_id && ret->getGuildId() == guild_sid && ret->getOneTimePassword() == one_time_password && ret->getGameServer()->getName() == gameServerName) {
			pendingRequests.erase(it);
		} else {
			ret = nullptr;
		}
	} else {
		ret = nullptr;
	}

	uv_mutex_unlock(&mapLock);

	return ret;
}

} // namespace UploadServer
