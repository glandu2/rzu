#pragma once

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

protected:
	virtual std::string getServerIp();
	virtual uint16_t getServerPort();

private:
	~AuthClientSession();
};

