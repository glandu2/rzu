#ifndef UPLOADSERVER_CLIENTSESSION_H
#define UPLOADSERVER_CLIENTSESSION_H

#include "PacketSession.h"
#include "EncryptedSession.h"

#include "Packets/TS_CU_LOGIN.h"
#include "Packets/TS_CU_UPLOAD.h"

namespace UploadServer {

class UploadRequest;

class ClientSession : public EncryptedSession<PacketSession>
{
	DECLARE_CLASS(UploadServer::ClientSession)

public:
	ClientSession();

protected:
	void onPacketReceived(const TS_MESSAGE* packet);

	void onLogin(const TS_CU_LOGIN* packet);
	void onUpload(const TS_CU_UPLOAD* packet);

	bool checkJpegImage(const char *data);

private:
	~ClientSession();

	UploadRequest* currentRequest;
};

}

#endif // UPLOADSERVER_CLIENTSESSION_H
