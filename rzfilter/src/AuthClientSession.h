#ifndef AUTHCLIENTSESSION_H
#define AUTHCLIENTSESSION_H

#include "ClientSession.h"
#include "GameClientSession.h"

class GameClientSessionManager;
class FilterManager;

class AuthClientSession : public ClientSession {
	DECLARE_CLASS(ClientSession)
public:
	struct InputParameters {
		GameClientSessionManager* gameClientSessionManager;
		FilterManager* filterManager;
		FilterManager* converterFilterManager;
	};

public:
	AuthClientSession(InputParameters parameters);

	virtual void onServerPacketReceived(const TS_MESSAGE* packet);

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

	virtual std::string getServerIp();
	virtual uint16_t getServerPort();

private:
	~AuthClientSession();
	GameClientSessionManager* gameClientSessionManager;
};

#endif  // CLIENTSESSION_H
