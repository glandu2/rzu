#ifndef UPLOADSERVER_GAMESERVERSESSION_H
#define UPLOADSERVER_GAMESERVERSESSION_H

#include "PacketSession.h"
#include "EncryptedSession.h"
#include <unordered_map>

#include "Packets/TS_SU_LOGIN.h"
#include "Packets/TS_SU_REQUEST_UPLOAD.h"

namespace UploadServer {

class GameServerSession : public EncryptedSession<PacketSession>
{
	DECLARE_CLASS(UploadServer::GameServerSession)
public:
	GameServerSession();

	void sendUploadResult(uint32_t guidId, uint32_t fileSize, const char *fileName);
	const std::string& getName() { return serverName; }

protected:
	~GameServerSession();

	void onPacketReceived(const TS_MESSAGE* packet);

	void onLogin(const TS_SU_LOGIN* packet);
	void onRequestUpload(const TS_SU_REQUEST_UPLOAD* packet);

	virtual void updateObjectName();

private:

	static std::unordered_map<std::string, GameServerSession*> servers;

	std::string serverName;
};

}

#endif // UPLOADSERVER_GAMESERVERSESSION_H
