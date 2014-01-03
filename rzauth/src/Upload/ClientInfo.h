#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include "Object.h"
#include "ICallbackGuard.h"
#include "RappelzSocket.h"
#include "UploadRequest.h"

#include "Packets/TS_CU_LOGIN.h"
#include "Packets/TS_CU_UPLOAD.h"

namespace UploadServer {

class ClientInfo : public Object, public ICallbackGuard
{
	DECLARE_CLASS(UploadServer::ClientInfo)

public:
	ClientInfo(RappelzSocket *socket);
	~ClientInfo();

	static void startServer();

protected:
	static void onNewConnection(ICallbackGuard* instance, Socket* serverSocket);
	static void onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState);
	static void onDataReceived(ICallbackGuard* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet);
	//static ClientData* popPendingClient(const std::string& accountName);

	void onLogin(const TS_CU_LOGIN* packet);
	void onUpload(const TS_CU_UPLOAD* packet);

private:
	RappelzSocket* socket;

	UploadRequest* currentRequest;
};

}

#endif // CLIENTINFO_H
