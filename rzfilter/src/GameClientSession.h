#ifndef GAMECLIENTSESSION_H
#define GAMECLIENTSESSION_H

#include "ClientSession.h"

class AuthClientSession;

class GameClientSession : public ClientSession
{
	DECLARE_CLASS(GameClientSession)

public:
	struct Parameter {
	    std::string serverIp;
	    uint16_t serverPort;
    };

public:
    GameClientSession(Parameter* serverAddress);

protected:
	virtual std::string getServerIp();
	virtual uint16_t getServerPort();

private:
	~GameClientSession();

	std::string serverIp;
	uint16_t serverPort;
};

#endif // GAMECLIENTSESSION_H
