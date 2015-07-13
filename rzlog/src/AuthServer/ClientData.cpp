#include "ClientData.h"
#include "uv.h"
#include "ClientSession.h"
#include "GameData.h"
#include <algorithm>
#include <time.h>

namespace AuthServer {

uv_mutex_t ClientData::mapLock = initializeLock();
std::unordered_map<uint32_t, ClientData*> ClientData::connectedClients;
std::unordered_map<std::string, ClientData*> ClientData::connectedClientsByName;

uv_mutex_t ClientData::initializeLock() {
	uv_mutex_init(&mapLock);
	return mapLock;
}

ClientData::ClientData(ClientSession *clientInfo)
	: accountId(0), kickRequested(false),
	  client(clientInfo), server(nullptr), inGame(false) {
}

ClientData::~ClientData() {
	if(getGameServer() && inGame)
		getGameServer()->decPlayerCount();
}


static int toLowerChar(int c) {
	if(c >= 'A' && c <= 'Z')
		c += 'a' - 'A';

	return c;
}

std::string ClientData::toLower(const std::string& str) {
	std::string lcase = str;
	std::transform(lcase.begin(), lcase.end(), lcase.begin(), toLowerChar);
	return lcase;
}

ClientData* ClientData::tryAddClient(ClientSession *clientInfo, const std::string& account, uint32_t accoundId, uint32_t age, uint32_t event_code, uint32_t pcBang, uint32_t ip, ClientData** oldClientPtr) {
	std::pair< std::unordered_map<uint32_t, ClientData*>::iterator, bool> result;
	std::pair< std::unordered_map<std::string, ClientData*>::iterator, bool> resultForName;
	ClientData* newClient;

	if(oldClientPtr)
		*oldClientPtr = nullptr;

	uv_mutex_lock(&mapLock);

	newClient = new ClientData(clientInfo);
	result = connectedClients.insert(std::pair<uint32_t, ClientData*>(accoundId, newClient));
	if(result.second == false) {
		ClientData* oldClient = result.first->second;
		delete newClient;
		newClient = nullptr;

		if(oldClientPtr)
			*oldClientPtr = oldClient;
	} else {
		newClient->account = account;
		newClient->accountId = accoundId;
		newClient->age = age;
		newClient->eventCode = event_code;
		newClient->pcBang = pcBang;
		newClient->ip = ip;
		resultForName = connectedClientsByName.insert(std::pair<std::string, ClientData*>(toLower(account), newClient));
		if(resultForName.second == false) {
			newClient->error("Duplicated account name with different ID: %s\n", account.c_str());
			if(oldClientPtr)
				*oldClientPtr = resultForName.first->second;
			connectedClients.erase(result.first);
			delete newClient;
			newClient = nullptr;
		}
	}

	uv_mutex_unlock(&mapLock);

	return newClient;
}

bool ClientData::removeClient(const std::string& account) {
	bool ret = false;
	std::unordered_map<std::string, ClientData*>::iterator it;

	uv_mutex_lock(&mapLock);
	it = connectedClientsByName.find(toLower(account));
	if(it != connectedClientsByName.end()) {
		ClientData* clientData = it->second;
		connectedClientsByName.erase(it);
		connectedClients.erase(clientData->accountId);
		delete clientData;
		ret = true;
	} else {
		Log::get()->log(Log::LL_Error, "ClientData", 10, "Trying to remove a not connected account : %s\n", account.c_str());
	}
	uv_mutex_unlock(&mapLock);

	return ret;
}

bool ClientData::removeClient(uint32_t accountId) {
	bool ret = false;
	std::unordered_map<uint32_t, ClientData*>::iterator it;

	uv_mutex_lock(&mapLock);
	it = connectedClients.find(accountId);
	if(it != connectedClients.end()) {
		ClientData* clientData = it->second;
		connectedClients.erase(it);
		connectedClientsByName.erase(toLower(clientData->account));
		delete clientData;
		ret = true;
	} else {
		Log::get()->log(Log::LL_Error, "ClientData", 10, "Trying to remove a not connected account : %d\n", accountId);
	}
	uv_mutex_unlock(&mapLock);

	return ret;
}

bool ClientData::removeClient(ClientData* clientData) {
	return removeClient(clientData->accountId);
}

void ClientData::switchClientToServer(GameData* server, uint64_t oneTimePassword) {
	this->oneTimePassword = oneTimePassword;
	this->client = nullptr;
	this->server = server;
}

void ClientData::connectedToGame() {
	if(!getGameServer())
		error("Connected to unknown game server ! Code logic error\n");
	else
		getGameServer()->incPlayerCount();
	inGame = true;
	loginTime = time(nullptr);
}

ClientData* ClientData::getClient(const std::string& account) {
	ClientData* foundClient;
	std::unordered_map<std::string, ClientData*>::const_iterator it;

	uv_mutex_lock(&mapLock);

	it = connectedClientsByName.find(toLower(account));
	if(it != connectedClientsByName.cend())
		foundClient = it->second;
	else
		foundClient = nullptr;

	uv_mutex_unlock(&mapLock);

	return foundClient;
}

ClientData* ClientData::getClientById(uint32_t accountId) {
	ClientData* foundClient;
	std::unordered_map<uint32_t, ClientData*>::const_iterator it;

	uv_mutex_lock(&mapLock);

	it = connectedClients.find(accountId);
	if(it != connectedClients.cend())
		foundClient = it->second;
	else
		foundClient = nullptr;

	uv_mutex_unlock(&mapLock);

	return foundClient;
}

void ClientData::removeServer(GameData* server) {
	std::unordered_map<uint32_t, ClientData*>::const_iterator it, itEnd;

	uv_mutex_lock(&mapLock);
	for(it = connectedClients.begin(), itEnd = connectedClients.end(); it != itEnd;) {
		ClientData* client = it->second;
		if(client->getGameServer() == server) {
			connectedClientsByName.erase(toLower(client->account));
			it = connectedClients.erase(it);
			delete client;
		} else {
			++it;
		}
	}
	uv_mutex_unlock(&mapLock);
}

} // namespace AuthServer
