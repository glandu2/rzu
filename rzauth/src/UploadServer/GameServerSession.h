#ifndef GAMESERVERINFO_H
#define GAMESERVERINFO_H

#include "RappelzSession.h"
#include <unordered_map>

#include "Packets/TS_SU_LOGIN.h"
#include "Packets/TS_SU_REQUEST_UPLOAD.h"

namespace UploadServer {

class GameServerSession : public RappelzSession
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

#endif // GAMESERVERINFO_H
