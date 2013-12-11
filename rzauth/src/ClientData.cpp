#include "ClientData.h"
#include "uv.h"

uv_rwlock_t ClientData::mapLock;
uv_once_t ClientData::lockInit = UV_ONCE_INIT;
std::unordered_map<std::string, ClientData*> ClientData::connectedClients;

void ClientData::initializeLock(){
	uv_rwlock_init(&mapLock);
}

ClientData::ClientData(ClientInfo *clientInfo) : accountId(0), client(clientInfo), server(nullptr) {
	uv_once(&lockInit, &initializeLock);
}

bool ClientData::tryAddClient(ClientData* newClient, ClientData** oldClient) {
	std::pair< std::unordered_map<std::string, ClientData*>::iterator, bool> result;
	uv_rwlock_wrlock(&mapLock);
	result = connectedClients.emplace(newClient->account, newClient);
	if(oldClient) *oldClient = result.first->second;
	uv_rwlock_wrunlock(&mapLock);

	return result.second;
}

bool ClientData::removeClient(const std::string& account) {
	bool ret;

	uv_rwlock_wrlock(&mapLock);
	ret = connectedClients.erase(account) > 0;
	uv_rwlock_wrunlock(&mapLock);

	return ret;
}

ClientData* ClientData::getClient(const std::string& account) {
	ClientData* foundClient;
	std::unordered_map<std::string, ClientData*>::const_iterator it;

	uv_rwlock_rdlock(&mapLock);
	it = connectedClients.find(account);
	if(it != connectedClients.cend())
		foundClient = it->second;
	else
		foundClient = nullptr;
	uv_rwlock_rdunlock(&mapLock);

	return foundClient;
}
