#ifndef AUTHSERVER_CLIENTSESSION_H
#define AUTHSERVER_CLIENTSESSION_H

#include "NetSession/PacketSession.h"
#include "NetSession/EncryptedSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"
#include "DB_Account.h"
#include "DB_UpdateLastServerIdx.h"

#include "AuthClient/TS_CA_VERSION.h"
#include "AuthClient/TS_CA_RSA_PUBLIC_KEY.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "AuthClient/TS_CA_IMBC_ACCOUNT.h"
#include "AuthClient/TS_CA_SERVER_LIST.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"

class IDbQueryJob;
class DesPasswordCipher;

namespace AuthServer {

class ClientSession : public EncryptedSession<PacketSession>
{
	DECLARE_CLASS(AuthServer::ClientSession)

public:
	ClientSession();

	void clientAuthResult(DB_Account *query);

protected:
	void onPacketReceived(const TS_MESSAGE* packet);

	void onVersion(const TS_CA_VERSION* packet);
	void onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet);
	void onAccount(const TS_CA_ACCOUNT* packet);
	void onImbcAccount(const TS_CA_IMBC_ACCOUNT* packet);
	void onServerList(const TS_CA_SERVER_LIST* packet);
	void onSelectServer(const TS_CA_SELECT_SERVER* packet);
	
private:
	~ClientSession();

	bool useRsaAuth;
	bool isEpic2;
	uint16_t lastLoginServerId;
	uint32_t serverIdxOffset;
	unsigned char aesKey[32];

	ClientData* clientData;
	DbQueryJobRef dbQuery;
};

} // namespace AuthServer

#endif // AUTHSERVER_CLIENTSESSION_H
