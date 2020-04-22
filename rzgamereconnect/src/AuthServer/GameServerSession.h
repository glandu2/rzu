#pragma once

#include "NetSession/PacketSession.h"
#include <stdint.h>
#include <string>
#include <unordered_map>

#include "AuthGame/TS_GA_CLIENT_KICK_FAILED.h"
#include "AuthGame/TS_GA_CLIENT_LOGIN.h"
#include "AuthGame/TS_GA_CLIENT_LOGOUT.h"
#include "AuthGame/TS_GA_LOGIN.h"

namespace AuthServer {

class AuthSession;

class GameServerSession : public PacketSession {
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

	// virtual void updateObjectName();

private:
	~GameServerSession();

	AuthSession* authSession;
	uint16_t serverIdx;
};

}  // namespace AuthServer

