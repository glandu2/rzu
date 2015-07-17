#ifndef GAMESERVER_AUTHSERVERSESSION_H
#define GAMESERVER_AUTHSERVERSESSION_H

#include "PacketSession.h"
#include "StartableObject.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"

namespace GameServer {

class AuthServerSession : public PacketSession, public StartableObject
{
	DECLARE_CLASS(GameServer::AuthServerSession)
public:
	static void init();
	static void deinit();

	cval<bool>& getAutoStartConfig();

	bool start();
	void stop() { closeSession(); }
	bool isStarted() { return getStream() && getStream()->getState() == Stream::ConnectedState; }

	AuthServerSession();
	~AuthServerSession();

	static AuthServerSession* get() { return instance; }

	void loginClient(const char* account);
	void logoutClient(const char* account);

protected:

	void onPacketReceived(const TS_MESSAGE* packet);

	void onConnected();
	void onLoginResult(const TS_AG_LOGIN_RESULT* packet);

	static AuthServerSession* instance;
};

}

#endif // GAMESERVER_AUTHSERVERSESSION_H
