#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include <string>
#include <unordered_map>
#include <stdint.h>
#include "uv.h"

class ClientInfo;
class ServerInfo;

class ClientData {
public:
	ClientData(ClientInfo* clientInfo);

	std::string account;
	uint32_t accountId;
	uint32_t age;
	uint16_t lastLoginServerId;
	uint32_t eventCode;
	uint64_t oneTimePassword;

	ClientInfo* client; //if != null: not yet in-game
	ServerInfo* server; //if != null: in-game

	//Try to add newClient if account is not already in the list (authenticated).
	//If another account with same name already exist, return false and return existing client pointer in oldClient
	//Thread safe
	static bool tryAddClient(ClientData* newClient, ClientData** oldClient);
	static bool removeClient(const std::string& account);
	static ClientData* getClient(const std::string& account);

private:
	static void initializeLock();

	static std::unordered_map<std::string, ClientData*> connectedClients;
	static uv_rwlock_t mapLock;
	static uv_once_t lockInit;
};

#endif // CLIENTDATA_H
