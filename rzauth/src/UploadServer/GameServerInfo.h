#ifndef GAMESERVERINFO_H
#define GAMESERVERINFO_H

#include "Object.h"
#include "ICallbackGuard.h"
#include "RappelzSocket.h"
#include <unordered_map>

#include "Packets/TS_SU_LOGIN.h"
#include "Packets/TS_SU_REQUEST_UPLOAD.h"

namespace UploadServer {

class GameServerInfo : public Object, public ICallbackGuard
{
	DECLARE_CLASS(UploadServer::GameServerInfo)
public:
	GameServerInfo(RappelzSocket* socket);
	~GameServerInfo();

	void sendUploadResult(uint32_t guidId, uint32_t fileSize, const char *fileName);
	const std::string& getName() { return serverName; }

	static void startServer();

protected:
	static void onNewConnection(ICallbackGuard* instance, Socket* serverSocket);
	static void onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState);
	static void onDataReceived(ICallbackGuard* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet);

	void onLogin(const TS_SU_LOGIN* packet);
	void onRequestUpload(const TS_SU_REQUEST_UPLOAD* packet);

private:
	static std::unordered_map<std::string, GameServerInfo*> servers;

	RappelzSocket* socket;

	std::string serverName;
};

}

#endif // GAMESERVERINFO_H
