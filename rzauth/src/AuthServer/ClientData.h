#ifndef AUTHSERVER_CLIENTDATA_H
#define AUTHSERVER_CLIENTDATA_H

#include "Object.h"
#include <string>
#include <unordered_map>
#include <stdint.h>
#include "uv.h"

namespace AuthServer {

class ClientSession;
class GameData;

class ClientData : public Object
{
	DECLARE_CLASS(AuthServer::ClientData)

public:
	ClientData(ClientSession* clientInfo);
	void switchClientToServer(GameData* server, uint64_t oneTimePassword);

	std::string account;
	uint32_t accountId;
	uint32_t age;
	uint32_t eventCode;
	uint64_t oneTimePassword;
	uint32_t pcBang;
	uint32_t ip;
	time_t loginTime;
	bool kickRequested;

	//Try to add newClient if account is not already in the list (authenticated).
	//There is at most one account in the hash map.
	//If the account is already in the hash map, fail: return null and put already connected client data in oldClient
	//If successful, create a new instance of ClientData with given account added to the hash map
	//Thread safe
	static ClientData* tryAddClient(ClientSession* clientInfo, const std::string& account, uint32_t accoundId, uint32_t age, uint32_t event_code, uint32_t pcBang, uint32_t ip, ClientData** oldClient = nullptr);
	static bool removeClient(uint32_t accountId);
	static bool removeClient(const std::string& account);
	static bool removeClient(ClientData* clientData);
	static ClientData* getClient(const std::string& account);
	static ClientData* getClientById(uint32_t accountId);
	static unsigned int getClientCount() { return (int)connectedClients.size(); }
	static void removeServer(GameData* server); //remove all client that was connected to this server


	void connectedToGame();
	bool isConnectedToGame() { return inGame; }

	ClientSession* getClientSession() { return client; }
	GameData* getGameServer() { return server; }

protected:
	static std::string toLower(const std::string& str);

private:
	~ClientData();

	static uv_mutex_t initializeLock();

	static std::unordered_map<uint32_t, ClientData*> connectedClients;
	static std::unordered_map<std::string, ClientData*> connectedClientsByName;
	static uv_mutex_t mapLock;


	ClientSession* client; //if != null: not yet in-game
	GameData* server; //if != null: in-game or gameserver selected
	//never both !client && !server
	bool inGame;
};

} // namespace AuthServer

#endif // AUTHSERVER_CLIENTDATA_H
