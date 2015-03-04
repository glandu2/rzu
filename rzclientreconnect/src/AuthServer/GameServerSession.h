#ifndef AUTHSERVER_GAMESERVERSESSION_H
#define AUTHSERVER_GAMESERVERSESSION_H

#include "PacketSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"
#include "AuthSession.h"

#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_KICK_FAILED.h"

namespace AuthServer {

class GameServerSession : public PacketSession
{
	DECLARE_CLASS(AuthServer::GameServerSession)

public:
	GameServerSession();

protected:
	void onPacketReceived(const TS_MESSAGE* packet);
	void onConnected();
	void onDisconnected(bool causedByRemote);

	void onServerLogin(const TS_GA_LOGIN* packet);
	void onClientLogin(const TS_GA_CLIENT_LOGIN* packet);
	void onClientLogout(const TS_GA_CLIENT_LOGOUT* packet);

	virtual void updateObjectName();

private:
	~GameServerSession();

	AuthSession* authSession;
	uint16_t serverIdx;
};

} // namespace AuthServer

#endif // AUTHSERVER_GAMESERVERSESSION_H
