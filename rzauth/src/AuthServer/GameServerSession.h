#ifndef AUTHSERVER_GAMESERVERSESSION_H
#define AUTHSERVER_GAMESERVERSESSION_H

#include "PacketSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"

#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_KICK_FAILED.h"

namespace AuthServer {

class GameData;

class GameServerSession : public PacketSession
{
	DECLARE_CLASS(AuthServer::GameServerSession)

public:
	GameServerSession();

	void kickClient(ClientData *clientData);
	void sendNotifyItemPurchased(ClientData* client);

protected:
	void onPacketReceived(const TS_MESSAGE* packet);

	void onServerLogin(const TS_GA_LOGIN* packet);
	void onServerLogout(const TS_GA_LOGOUT* packet);
	void onClientLogin(const TS_GA_CLIENT_LOGIN* packet);
	void onClientLogout(const TS_GA_CLIENT_LOGOUT* packet);
	void onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet);

private:
	~GameServerSession();

	bool useAutoReconnectFeature;
	GameData* gameData;
};

} // namespace AuthServer

#endif // AUTHSERVER_GAMESERVERSESSION_H
