#pragma once

#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "AuthGame/TS_AG_LOGIN_RESULT.h"
#include "NetSession/PacketSession.h"
#include "NetSession/StartableObject.h"

namespace GameServer {

class ClientSession;

class AuthServerSession : public PacketSession, public StartableObject {
	DECLARE_CLASS(GameServer::AuthServerSession)
public:
	static void init();
	static void deinit();

	cval<bool>& getAutoStartConfig();

	bool start() override;
	void stop() override { closeSession(); }
	bool isStarted() override { return getStream() && getStream()->getState() == Stream::ConnectedState; }

	AuthServerSession();
	~AuthServerSession();

	static AuthServerSession* get() { return instance; }

	void loginClient(ClientSession* clientSession, const std::string& account, uint64_t oneTimePassword);
	void logoutClient(const char* account, uint32_t playTime);

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet) override;
	EventChain<SocketSession> onConnected() override;

	void onLoginResult(const TS_AG_LOGIN_RESULT* packet);
	void onClientLoginResult(const TS_AG_CLIENT_LOGIN* packet);

	static AuthServerSession* instance;
	std::unordered_map<std::string, ClientSession*> pendingClients;
};

}  // namespace GameServer

