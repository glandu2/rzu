#pragma once

#include "Cipher/AesPasswordCipher.h"
#include "GameClientSession.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include <stdint.h>
#include <string>

struct TS_CA_VERSION;
struct TS_CA_RSA_PUBLIC_KEY;
struct TS_CA_ACCOUNT;
struct TS_CA_SERVER_LIST;
struct TS_CA_SELECT_SERVER;

class AuthClientSession : public EncryptedSession<PacketSession> {
	DECLARE_CLASS(AuthClientSession)

public:
	AuthClientSession();

protected:
	virtual EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

	void onVersion(const TS_CA_VERSION* packet);

	void onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet);
	void onAccount(const TS_CA_ACCOUNT* packet);
	void onServerList(const TS_CA_SERVER_LIST* packet);
	void onSelectServer(const TS_CA_SELECT_SERVER* packet);

private:
	~AuthClientSession();

	std::string account;

	AesPasswordCipher clientAesCipher;
};

