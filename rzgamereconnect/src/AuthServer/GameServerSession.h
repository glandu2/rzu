#ifndef AUTHSERVER_GAMESERVERSESSION_H
#define AUTHSERVER_GAMESERVERSESSION_H

#include "NetSession/PacketSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>

#include "AuthGame/TS_GA_LOGIN.h"
#include "AuthGame/TS_GA_CLIENT_LOGIN.h"
#include "AuthGame/TS_GA_CLIENT_LOGOUT.h"
#include "AuthGame/TS_GA_CLIENT_KICK_FAILED.h"

namespace AuthServer {

class AuthSession;

class GameServerSession : public PacketSession
{
	DECLARE_CLASS(AuthServer::GameServerSession)

public:
	GameServerSession();

	void disconnectAuth();

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);
	EventChain<SocketSession> onDisconnected(bool causedByRemote);

	void onServerLogin(const TS_GA_LOGIN* packet);
	void onClientLogin(const TS_GA_CLIENT_LOGIN* packet);
	void onClientLogout(const TS_GA_CLIENT_LOGOUT* packet);

	//virtual void updateObjectName();

private:
	~GameServerSession();

	AuthSession* authSession;
	uint16_t serverIdx;
};

} // namespace AuthServer

#endif // AUTHSERVER_GAMESERVERSESSION_H
