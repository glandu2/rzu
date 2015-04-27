#include "ClientSession.h"
#include "GameServerSession.h"
#include "UploadRequest.h"
#include "../GlobalConfig.h"
#include "PrintfFormats.h"

#include <time.h>
#include <stdio.h>
#include "Utils.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_UC_LOGIN_RESULT.h"
#include "Packets/TS_UC_UPLOAD.h"

namespace UploadServer {

ClientSession::ClientSession() {
	currentRequest = nullptr;
}

ClientSession::~ClientSession() {
}

void ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_CU_LOGIN::packetID:
			onLogin(static_cast<const TS_CU_LOGIN*>(packet));
			break;

		case TS_CU_UPLOAD::packetID:
			onUpload(static_cast<const TS_CU_UPLOAD*>(packet));
			break;

		default:
			debug("Unknown packet ID: %d, size: %d\n", packet->id, packet->size);
			break;
	}
}

void ClientSession::onLogin(const TS_CU_LOGIN* packet) {
	TS_UC_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_UC_LOGIN_RESULT>(&result);

	std::string serverName = Utils::convertToString(packet->raw_server_name, sizeof(packet->raw_server_name)-1);

	debug("Upload from client %s:%d, client id %u with account id %u for guild id %u on server %s\n",
		  getStream()->getRemoteIpStr(),
		  getStream()->getRemotePort(),
		  packet->client_id,
		  packet->account_id,
		  packet->guild_id,
		  serverName.c_str());

	currentRequest = UploadRequest::popRequest(packet->client_id, packet->account_id, packet->guild_id, packet->one_time_password, serverName);

	if(currentRequest) {
		result.result = TS_RESULT_SUCCESS;
	} else {
		debug("Invalid client, otp given is %u\n", packet->one_time_password);
		result.result = TS_RESULT_INVALID_ARGUMENT;
	}

	sendPacket(&result);
}

void ClientSession::onUpload(const TS_CU_UPLOAD* packet) {
	TS_UC_UPLOAD result;
	TS_MESSAGE::initMessage<TS_UC_UPLOAD>(&result);


	debug("Upload from client %s:%d\n", getStream()->getRemoteIpStr(), getStream()->getRemotePort());

	if(currentRequest == nullptr) {
		warn("Upload attempt without a request from %s:%d\n", getStream()->getRemoteIpStr(), getStream()->getRemotePort());
		result.result = TS_RESULT_NOT_EXIST;
	} else if(packet->file_length != packet->size - sizeof(TS_CU_UPLOAD)) {
		warn("Upload packet size invalid, received %d bytes but the packet say %u bytes\n", int(packet->size - sizeof(TS_CU_UPLOAD)), packet->file_length);
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

	sendPacket(&result);
}

bool ClientSession::checkJpegImage(const char *data) {
	if(*(const uint16_t*)data != 0xFFD8)
		return false;

	return true;
}

} // namespace UploadServer
