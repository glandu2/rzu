#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include "RappelzSession.h"
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

class DB_Account;

class ClientSession : public RappelzSession
{
	DECLARE_CLASS(AuthServer::ClientSession)

public:
	ClientSession();

	void clientAuthResult(bool authOk, const std::string& account, uint32_t accountId, uint32_t age, uint16_t lastLoginServerIdx, uint32_t eventCode);

protected:
	void onPacketReceived(const TS_MESSAGE* packet);

	void onVersion(const TS_CA_VERSION* packet);
	void onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet);
	void onAccount(const TS_CA_ACCOUNT* packet);
	void onServerList(const TS_CA_SERVER_LIST* packet);
	void onServerList_epic2(const TS_CA_SERVER_LIST* packet);
	void onSelectServer(const TS_CA_SELECT_SERVER* packet);
	
private:
	~ClientSession();

	enum AuthMethod {
		AM_DES,
		AM_RSA,
		AM_DES_E4
	};

	bool useRsaAuth;
	bool isEpic2;
	unsigned char aesKey[32];
	uint16_t lastLoginServerId;
	ClientData* clientData;
	DB_Account* dbQuery;
};

} // namespace AuthServer

#endif // CLIENTSESSION_H
