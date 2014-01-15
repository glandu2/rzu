#include "ClientSession.h"
#include "RappelzSocket.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include "Packets/PacketEnums.h"
#include "GameServerSession.h"
#include <time.h>
#include <stdio.h>
#include "Utils.h"

#include "Packets/TS_UC_LOGIN_RESULT.h"
#include "Packets/TS_UC_UPLOAD.h"

namespace UploadServer {

ClientSession::ClientSession(RappelzSocket* socket) {
	this->socket = socket;
	currentRequest = nullptr;

	socket->addEventListener(this, &onStateChanged);
	socket->addPacketListener(TS_CU_LOGIN::packetID, this, &onDataReceived);
	socket->addPacketListener(TS_CU_UPLOAD::packetID, this, &onDataReceived);
}

ClientSession::~ClientSession() {
	socket->deleteLater();
}

void ClientSession::startServer() {
	Socket* serverSocket = new Socket(EventLoop::getLoop());
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen(CONFIG_GET()->upload.client.listenIp,
						 CONFIG_GET()->upload.client.port);
}

void ClientSession::onNewConnection(IListener* instance, Socket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket(EventLoop::getLoop(), true);
	static ClientSession* clientInfo = new ClientSession(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		newSocket = new RappelzSocket(EventLoop::getLoop(), true);
		clientInfo = new ClientSession(newSocket);
	} while(1);
}

void ClientSession::onStateChanged(IListener* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	ClientSession* thisInstance = static_cast<ClientSession*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void ClientSession::onDataReceived(IListener* instance, RappelzSocket*, const TS_MESSAGE* packet) {
	ClientSession* thisInstance = static_cast<ClientSession*>(instance);

	switch(packet->id) {
		case TS_CU_LOGIN::packetID:
			thisInstance->onLogin(static_cast<const TS_CU_LOGIN*>(packet));
			break;

		case TS_CU_UPLOAD::packetID:
			thisInstance->onUpload(static_cast<const TS_CU_UPLOAD*>(packet));
			break;
	}
}

void ClientSession::onLogin(const TS_CU_LOGIN* packet) {
	TS_UC_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_UC_LOGIN_RESULT>(&result);

	debug("Upload from client %s:%d, client id %u with account id %u for guild id %u on server %30s\n",
		  socket->getHost().c_str(),
		  socket->getPort(),
		  packet->client_id,
		  packet->account_id,
		  packet->guild_id,
		  packet->raw_server_name);

	currentRequest = UploadRequest::popRequest(packet->client_id, packet->account_id, packet->guild_id, packet->one_time_password, packet->raw_server_name);

	if(currentRequest)
		result.result = TS_RESULT_SUCCESS;
	else {
		debug("Invalid client, otp given is %u\n", packet->one_time_password);
		result.result = TS_RESULT_INVALID_ARGUMENT;
	}

	socket->sendPacket(&result);
}

void ClientSession::onUpload(const TS_CU_UPLOAD* packet) {
	TS_UC_UPLOAD result;
	TS_MESSAGE::initMessage<TS_UC_UPLOAD>(&result);


	debug("Upload from client %s:%d\n", socket->getHost().c_str(), socket->getPort());

	if(currentRequest == nullptr) {
		warn("Upload attempt without a request from %s:%d\n", socket->getHost().c_str(), socket->getPort());
		result.result = TS_RESULT_NOT_EXIST;
	} else if(packet->file_length != packet->size - sizeof(TS_CU_UPLOAD)) {
		warn("Upload packet size invalid, received %u bytes but the packet say %u bytes\n", packet->size - sizeof(TS_CU_UPLOAD), packet->file_length);
		result.result = TS_RESULT_INVALID_ARGUMENT;
	} else if(packet->file_length > 64000) {
		debug("Upload file is too large: %d bytes. Max 64000 bytes\n", packet->file_length);
		result.result = TS_RESULT_LIMIT_MAX;
	} else if(!checkJpegImage(packet->file_contents)) {
		debug("Upload file is not a jpeg file\n");
		result.result = TS_RESULT_INVALID_ARGUMENT;
	} else {
		int filenameSize = currentRequest->getGameServer()->getName().size() + 1 + 10 + 1 + 2 + 2 + 2 + 1 + 2 + 2 + 2 + 4 + 1;
		char *filename = (char*)alloca(filenameSize);
		struct tm timeinfo;

		Utils::getGmTime(time(NULL), &timeinfo);

		snprintf(filename, filenameSize, "%s_%010d_%02d%02d%02d_%02d%02d%02d.jpg",
				 currentRequest->getGameServer()->getName().c_str(),
				 currentRequest->getGuildId(),
				 timeinfo.tm_year % 100,
				 timeinfo.tm_mon,
				 timeinfo.tm_mday,
				 timeinfo.tm_hour,
				 timeinfo.tm_min,
				 timeinfo.tm_sec);

		debug("Uploading image %s for client id %u with account id %u for guild %u\n", filename, currentRequest->getClientId(), currentRequest->getAccountId(), currentRequest->getGuildId());

		std::string absoluteDir = CONFIG_GET()->upload.client.uploadDir.get();
		std::string fullFileName = absoluteDir + "/" + filename;

		Utils::mkdir(absoluteDir.c_str());
		FILE* file = fopen(fullFileName.c_str(), "wb");
		if(!file) {
			warn("Cant open upload target file %s\n", fullFileName.c_str());
			result.result = TS_RESULT_ACCESS_DENIED;
		} else {
			int dataWritten = fwrite(&packet->file_contents, packet->file_length, 1, file);
			fclose(file);

			if(dataWritten == 1) {
				result.result = TS_RESULT_SUCCESS;
				currentRequest->getGameServer()->sendUploadResult(currentRequest->getGuildId(), packet->file_length, filename);
			} else {
				warn("Cant write upload target file %s, disk full ?\n", fullFileName.c_str());
				result.result = TS_RESULT_ACCESS_DENIED;
			}
		}
	}

	socket->sendPacket(&result);
}

bool ClientSession::checkJpegImage(const char *data) {
	if(memcmp(&data[6], "JFIF", 4))
		return false;

	return true;
}

} // namespace UploadServer