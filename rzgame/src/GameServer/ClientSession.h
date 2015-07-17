#ifndef GAMESERVER_CLIENTSESSION_H
#define GAMESERVER_CLIENTSESSION_H

#include "PacketSession.h"
#include "EncryptedSession.h"
#include <unordered_map>

#include "Packets/TS_CS_ACCOUNT_WITH_AUTH.h"

namespace GameServer {

class ClientSession : public EncryptedSession<PacketSession>
{
	DECLARE_CLASS(GameServer::ClientSession)
public:
	static void init();
	static void deinit();

	ClientSession();

protected:
	~ClientSession();

	void onPacketReceived(const TS_MESSAGE* packet);

	void onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH* packet);
};

}

#endif // GAMESERVER_CLIENTSESSION_H
