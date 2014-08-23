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

class IDbQueryJob;

namespace AuthServer {

class DesPasswordCipher;

class ClientSession : public RappelzSession
{
	DECLARE_CLASS(AuthServer::ClientSession)

public:
	ClientSession();

	static void init(cval<std::string> &str);
	static void deinit();

	void clientAuthResult(bool authOk, const std::string& account, uint32_t accountId, uint32_t age, uint16_t lastLoginServerIdx, uint32_t eventCode, uint32_t pcBang, uint32_t serverMask);

protected:
	void onPacketReceived(const TS_MESSAGE* packet);

	void onVersion(const TS_CA_VERSION* packet);
	void onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet);
	void onAccount(const TS_CA_ACCOUNT* packet);
	void onServerList(const TS_CA_SERVER_LIST* packet);
	void onServerList_epic2(const TS_CA_SERVER_LIST* packet);
	void onSelectServer(const TS_CA_SELECT_SERVER* packet);

	static void updateDesKey(IListener* instance, cval<std::string>* str);
	
private:
	~ClientSession();

	bool useRsaAuth;
	bool isEpic2;
	uint16_t lastLoginServerId;
	uint32_t serverIdxOffset;
	unsigned char aesKey[32];

	ClientData* clientData;
	IDbQueryJob* dbQuery;

	static DesPasswordCipher* desCipher; //cached DES cipher
	static std::string currentDesKey;
};

} // namespace AuthServer

#endif // CLIENTSESSION_H
