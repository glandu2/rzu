#pragma once

#include "DB_Account.h"
#include "DB_UpdateLastServerIdx.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include <stdint.h>

struct TS_CA_VERSION;
struct TS_CA_RSA_PUBLIC_KEY;
struct TS_CA_ACCOUNT;
struct TS_CA_IMBC_ACCOUNT;
struct TS_CA_SERVER_LIST;
struct TS_CA_SELECT_SERVER;

namespace AuthServer {

class ClientData;

class ClientSession : public EncryptedSession<PacketSession> {
	DECLARE_CLASS(AuthServer::ClientSession)

public:
	ClientSession();

	void clientAuthResult(DB_Account* query);

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);
	EventChain<SocketSession> onDisconnected(bool causedByRemote);

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

}  // namespace AuthServer

