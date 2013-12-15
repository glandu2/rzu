#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "Network/Object.h"
#include "Network/RappelzSocket.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"

#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_CA_RSA_PUBLIC_KEY.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_SERVER_LIST.h"
#include "Packets/TS_CA_SELECT_SERVER.h"

class ClientInfo : public Object, public ICallbackGuard
{
	DECLARE_CLASS(ClientInfo)

public:
	ClientInfo(RappelzSocket *socket);
	~ClientInfo();

	static void startServer();

	void clientAuthResult(bool authOk, const std::string& account, uint32_t accountId, uint32_t age, uint16_t lastLoginServerIdx, uint32_t eventCode);

protected:
	static void onNewConnection(void* instance, Socket* serverSocket);
	static void onStateChanged(void* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState);
	static void onDataReceived(void* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet);
	//static ClientData* popPendingClient(const std::string& accountName);

	void onVersion(const TS_CA_VERSION* packet);
	void onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet);
	void onAccount(const TS_CA_ACCOUNT* packet);
	void onServerList(const TS_CA_SERVER_LIST* packet);
	void onSelectServer(const TS_CA_SELECT_SERVER* packet);
	
private:
	RappelzSocket* socket;

	bool useRsaAuth;
	unsigned char aesKey[32];
	ClientData* clientData;
};

#endif // CLIENTMANAGER_H