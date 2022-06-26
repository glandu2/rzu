#pragma once

#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"

#include "UploadClient/flat/TS_CU_LOGIN.h"
#include "UploadClient/flat/TS_CU_UPLOAD.h"

namespace UploadServer {

class UploadRequest;

class ClientSession : public EncryptedSession<PacketSession> {
	DECLARE_CLASS(UploadServer::ClientSession)

public:
	ClientSession();

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

	void onLogin(const TS_CU_LOGIN* packet);
	void onUpload(const TS_CU_UPLOAD* packet);

	bool checkJpegImage(uint32_t length, const unsigned char* data);

private:
	~ClientSession();

	UploadRequest* currentRequest;
};

}  // namespace UploadServer
