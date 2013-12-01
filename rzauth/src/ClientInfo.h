#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

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

class ClientInfo : public ICallbackGuard
{
public:
	ClientInfo(RappelzSocket *socket);
	~ClientInfo();

	static void startServer();
	static ClientData* popPendingClient(const std::string& accountName);

protected:
	static void onNewConnection(void* instance, ISocket* serverSocket);
	static void onStateChanged(void* instance, ISocket* clientSocket, ISocket::State oldState, ISocket::State newState);
	static void onDataReceived(void* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet);

	void onVersion(const TS_CA_VERSION* packet);
	void onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet);
	void onAccount(const TS_CA_ACCOUNT* packet);
	void onServerList(const TS_CA_SERVER_LIST* packet);
	void onSelectServer(const TS_CA_SELECT_SERVER* packet);
	
private:
	static ISocket* serverSocket;
	static std::unordered_map<std::string, ClientData*> pendingClients;	//Client that should go to a gameserver (added at TS_AC_ACCOUNT, removed when disconnected or selected a server)

	RappelzSocket* socket;

	bool useRsaAuth;
	unsigned char aesKey[32];
	ClientData* clientData;
};

#endif // CLIENTMANAGER_H
