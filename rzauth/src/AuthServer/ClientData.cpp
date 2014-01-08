#include "ClientData.h"
#include "uv.h"


namespace AuthServer {

uv_mutex_t ClientData::mapLock = initializeLock();
uv_once_t ClientData::lockInit = UV_ONCE_INIT;
std::unordered_map<std::string, ClientData*> ClientData::connectedClients;

uv_mutex_t ClientData::initializeLock() {
	uv_mutex_init(&mapLock);
	return mapLock;
}

ClientData::ClientData(ClientSession *clientInfo) : accountId(0), client(clientInfo), server(nullptr), inGame(false) {

}

ClientData* ClientData::tryAddClient(ClientSession *clientInfo, const std::string& account, ClientData** oldClient) {
	std::pair< std::unordered_map<std::string, ClientData*>::iterator, bool> result;
	ClientData* newClient;

	uv_mutex_lock(&mapLock);

	newClient = new ClientData(clientInfo);
	result = connectedClients.insert(std::pair<std::string, ClientData*>(account, newClient));
	if(result.second == false) {
		if(oldClient) *oldClient = result.first->second;
		delete newClient;
		newClient = nullptr;
	} else {
		if(oldClient) *oldClient = nullptr;
		newClient->account = account;
	}

	uv_mutex_unlock(&mapLock);

	return newClient;
}

bool ClientData::removeClient(const std::string& account) {
	bool ret;

	uv_mutex_lock(&mapLock);
	ret = connectedClients.erase(account) > 0;
	uv_mutex_unlock(&mapLock);

	return ret;
}

bool ClientData::switchClientToServer(const std::string& account, GameServerSession* server) {
	bool ret = false;
	std::unordered_map<std::string, ClientData*>::const_iterator it;

	uv_mutex_lock(&mapLock);

	it = connectedClients.find(account);
	if(it != connectedClients.cend()) {
		ClientData* clientData = it->second;
		clientData->client = nullptr;
		clientData->server = server;
		ret = true;
	}
	uv_mutex_unlock(&mapLock);

	return ret;
}

ClientData* ClientData::getClient(const std::string& account) {
	ClientData* foundClient;
	std::unordered_map<std::string, ClientData*>::const_iterator it;

	uv_mutex_lock(&mapLock);

	it = connectedClients.find(account);
	if(it != connectedClients.cend())
		foundClient = it->second;
	else
		foundClient = nullptr;

	uv_mutex_unlock(&mapLock);

	return foundClient;
}

} // namespace AuthServer
