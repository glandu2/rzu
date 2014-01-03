#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "Object.h"
#include "RappelzSocket.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"

#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_CA_RSA_PUBLIC_KEY.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_SERVER_LIST.h"
#include "Packets/TS_CA_SELECT_SERVER.h"

namespace AuthServer {

class ClientInfo : public Object, public ICallbackGuard
{
	DECLARE_CLASS(AuthServer::ClientInfo)

public:
	ClientInfo(RappelzSocket *socket);
	~ClientInfo();

	static void startServer();

	void clientAuthResult(bool authOk, const std::string& account, uint32_t accountId, uint32_t age, uint16_t lastLoginServerIdx, uint32_t eventCode);

protected:
	static void onNewConnection(ICallbackGuard* instance, Socket* serverSocket);
	static void onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState);
	static void onDataReceived(ICallbackGuard* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet);
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

} // namespace AuthServer

#endif // CLIENTMANAGER_H
