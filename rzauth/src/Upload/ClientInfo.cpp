#include "ClientInfo.h"
#include "RappelzSocket.h"
#include <time.h>
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include "Packets/PacketEnums.h"

#include "Packets/TS_UC_LOGIN_RESULT.h"
#include "Packets/TS_UC_UPLOAD.h"

namespace UploadServer {

ClientInfo::ClientInfo(RappelzSocket* socket) {
	this->socket = socket;
	currentRequest = nullptr;

	socket->addEventListener(this, &onStateChanged);
	socket->addPacketListener(TS_CU_LOGIN::packetID, this, &onDataReceived);
	socket->addPacketListener(TS_CU_UPLOAD::packetID, this, &onDataReceived);
}

ClientInfo::~ClientInfo() {
	socket->deleteLater();
}

void ClientInfo::startServer() {
	Socket* serverSocket = new Socket(EventLoop::getLoop());
	srand((unsigned int)time(NULL));
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen(CONFIG_GET()->uploadClientConfig.listenIp,
						 CONFIG_GET()->uploadClientConfig.port);
}

void ClientInfo::onNewConnection(ICallbackGuard* instance, Socket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket(EventLoop::getLoop(), true);
	static ClientInfo* clientInfo = new ClientInfo(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		newSocket = new RappelzSocket(EventLoop::getLoop(), true);
		clientInfo = new ClientInfo(newSocket);
	} while(1);
}

void ClientInfo::onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void ClientInfo::onDataReceived(ICallbackGuard* instance, RappelzSocket*, const TS_MESSAGE* packet) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);

	switch(packet->id) {
		case TS_CU_LOGIN::packetID:
			thisInstance->onLogin(static_cast<const TS_CU_LOGIN*>(packet));
			break;

		case TS_CU_UPLOAD::packetID:
			thisInstance->onUpload(static_cast<const TS_CU_UPLOAD*>(packet));
			break;
	}
}

void ClientInfo::onLogin(const TS_CU_LOGIN* packet) {
	TS_UC_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_UC_LOGIN_RESULT>(&result);

	currentRequest = UploadRequest::popRequest(packet->client_id, packet->account_id, packet->guild_id, packet->one_time_password, packet->raw_server_name);

	if(currentRequest)
		result.result = TS_RESULT_SUCCESS;
	else
		result.result = TS_RESULT_INVALID_ARGUMENT;

	socket->sendPacket(&result);
}

void ClientInfo::onUpload(const TS_CU_UPLOAD* packet) {
	TS_UC_UPLOAD result;
	TS_MESSAGE::initMessage<TS_UC_UPLOAD>(&result);

	debug("Upload from client %s:%d\n", socket->getHost().c_str(), socket->getPort());

	if(currentRequest == nullptr) {
		warn("Upload attempt without a request from %s:%d\n", socket->getHost().c_str(), socket->getPort());
		result.result = TS_RESULT_NOT_EXIST;
	} else if(packet->file_length > 64000) {
		debug("Upload file is too large: %d bytes. Max 64000 bytes\n", packet->file_length);
		result.result = TS_RESULT_LIMIT_MAX;
	} else {
		debug("Uploading image for client id %u with account id %u for guild %u\n", currentRequest->getClientId(), currentRequest->getAccountId(), currentRequest->getGuildId());
		//check image validity
		result.result = TS_RESULT_SUCCESS;
	}

	socket->sendPacket(&result);
}

} // namespace UploadServer
