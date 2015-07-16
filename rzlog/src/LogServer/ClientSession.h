#ifndef UPLOADSERVER_CLIENTSESSION_H
#define UPLOADSERVER_CLIENTSESSION_H

#include "LogPacketSession.h"

#include "LS_11N4S.h"

namespace LogServer {

class UploadRequest;

class ClientSession : public LogPacketSession
{
	DECLARE_CLASS(LogServer::ClientSession)

public:
	ClientSession();

protected:
	void onPacketReceived(const LS_11N4S* packet);

private:
	~ClientSession();
	FILE* file;
};

}

#endif // UPLOADSERVER_CLIENTSESSION_H
