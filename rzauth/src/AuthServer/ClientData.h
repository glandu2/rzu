#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include "Object.h"
#include <string>
#include <unordered_map>
#include <stdint.h>
#include "uv.h"

namespace AuthServer {

class ClientInfo;
class ServerInfo;

class ClientData : public Object
{
	DECLARE_CLASS(AuthServer::ClientData)

public:
	ClientData(ClientInfo* clientInfo);

	std::string account;
	uint32_t accountId;
	uint32_t age;
	uint16_t lastLoginServerId;
	uint32_t eventCode;
	uint64_t oneTimePassword;

	ClientInfo* client; //if != null: not yet in-game
	ServerInfo* server; //if != null: in-game or gameserver selected
	bool inGame;

	//Try to add newClient if account is not already in the list (authenticated).
	//There is at most one account in the hash map.
	//If the account is already in the hash map, fail: return null and put already connected client data in oldClient
	//If successful, create a new instance of ClientData with given account added to the hash map
	//Thread safe
	static ClientData* tryAddClient(ClientInfo* clientInfo, const std::string& account, ClientData** oldClient);
	static bool removeClient(const std::string& account);
	static bool switchClientToServer(const std::string& account, ServerInfo* server);
	static ClientData* getClient(const std::string& account);
	static unsigned int getClientCount() { return connectedClients.size(); }

private:
	static uv_mutex_t initializeLock();

	static std::unordered_map<std::string, ClientData*> connectedClients;
	static uv_mutex_t mapLock;
	static uv_once_t lockInit;
};

} // namespace AuthServer

#endif // CLIENTDATA_H
