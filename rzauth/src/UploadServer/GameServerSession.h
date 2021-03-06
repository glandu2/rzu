#pragma once

#include "NetSession/PacketSession.h"
#include <unordered_map>

#include "UploadGame/TS_SU_LOGIN.h"
#include "UploadGame/TS_SU_REQUEST_UPLOAD.h"

namespace UploadServer {

class GameServerSession : public PacketSession {
	DECLARE_CLASS(UploadServer::GameServerSession)
public:
	GameServerSession();

	void sendUploadResult(uint32_t guidId, uint32_t fileSize, const char* fileName);
	const std::string& getName() { return serverName; }

protected:
	~GameServerSession();

	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

	void onLogin(const TS_SU_LOGIN* packet);
	void onRequestUpload(const TS_SU_REQUEST_UPLOAD* packet);

private:
	static std::unordered_map<std::string, GameServerSession*> servers;

	std::string serverName;
};

}  // namespace UploadServer

