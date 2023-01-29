#include "ClientSession.h"
#include "../GlobalConfig.h"
#include "Core/PrintfFormats.h"
#include "GameServerSession.h"
#include "IconServerSession.h"
#include "UploadRequest.h"
#include <memory>

#include "Core/Utils.h"
#include <stdio.h>
#include <time.h>

#include "PacketEnums.h"
#include "UploadClient/flat/TS_UC_DOWNLOAD_ICON.h"
#include "UploadClient/flat/TS_UC_LOGIN_RESULT.h"
#include "UploadClient/flat/TS_UC_UPLOAD.h"

namespace UploadServer {

ClientSession::ClientSession()
    : EncryptedSession<PacketSession>(SessionType::UploadClient, SessionPacketOrigin::Server, EPIC_LATEST) {
	currentRequest = nullptr;
}

ClientSession::~ClientSession() {
	if(currentRequest)
		delete currentRequest;
}

EventChain<PacketSession> ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_CU_LOGIN::packetID:
			onLogin(static_cast<const TS_CU_LOGIN*>(packet));
			break;

		case TS_CU_UPLOAD::packetID:
			onUpload(static_cast<const TS_CU_UPLOAD*>(packet));
			break;

		case TS_CU_DOWNLOAD_ICON::packetID:
			onDownload(static_cast<const TS_CU_DOWNLOAD_ICON*>(packet));
			break;

		default:
			log(LL_Debug, "Unknown packet ID: %d, size: %d\n", packet->id, packet->size);
			break;
	}

	return PacketSession::onPacketReceived(packet);
}

void ClientSession::onLogin(const TS_CU_LOGIN* packet) {
	TS_UC_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_UC_LOGIN_RESULT>(&result);

	std::string serverName = Utils::convertToString(packet->raw_server_name, sizeof(packet->raw_server_name) - 1);
	StreamAddress remoteAddress = getStream()->getRemoteAddress();
	char ip[INET6_ADDRSTRLEN];

	remoteAddress.getName(ip, sizeof(ip));

	log(LL_Debug,
	    "Upload from client %s:%d, client id %u with account id %u for guild id %u on server %s\n",
	    ip,
	    remoteAddress.port,
	    packet->client_id,
	    packet->account_id,
	    packet->guild_id,
	    serverName.c_str());

	currentRequest = UploadRequest::popRequest(
	    packet->client_id, packet->account_id, packet->guild_id, packet->one_time_password, serverName);

	if(currentRequest) {
		result.result = TS_RESULT_SUCCESS;
	} else {
		log(LL_Debug, "Invalid client, otp given is %u\n", packet->one_time_password);
		result.result = TS_RESULT_NOT_EXIST;
	}

	sendPacket(&result);
}

void ClientSession::onUpload(const TS_CU_UPLOAD* packet) {
	TS_UC_UPLOAD result;
	char filename[128];
	StreamAddress remoteAddress = getStream()->getRemoteAddress();
	char ip[INET6_ADDRSTRLEN];

	TS_MESSAGE::initMessage<TS_UC_UPLOAD>(&result);
	remoteAddress.getName(ip, sizeof(ip));

	log(LL_Debug, "Upload from client %s:%d\n", ip, remoteAddress.port);

	if(currentRequest == nullptr) {
		log(LL_Warning, "Upload attempt without a request from %s:%d\n", ip, remoteAddress.port);
		result.result = TS_RESULT_NOT_EXIST;
	} else if(packet->file_length != packet->size - sizeof(TS_CU_UPLOAD)) {
		log(LL_Warning,
		    "Upload packet size invalid, received %d bytes but the packet say %u bytes\n",
		    int(packet->size - sizeof(TS_CU_UPLOAD)),
		    packet->file_length);
		result.result = TS_RESULT_INVALID_ARGUMENT;
	} else if(packet->file_length > 64000) {
		log(LL_Debug, "Upload file is too large: %d bytes. Max 64000 bytes\n", packet->file_length);
		result.result = TS_RESULT_LIMIT_MAX;
	} else if(!checkJpegImage(packet->file_length, packet->file_contents)) {
		log(LL_Debug, "Upload file is not a jpeg file\n");
		result.result = TS_RESULT_INVALID_ARGUMENT;
	} else if(currentRequest->getGameServer()->getName().size() > sizeof(filename) / 2) {
		log(LL_Debug, "Game server name is too long: %d\n", (int) currentRequest->getGameServer()->getName().size());
		result.result = TS_RESULT_INVALID_TEXT;
	} else {
		struct tm timeinfo;

		Utils::getGmTime(time(nullptr), &timeinfo);

		snprintf(filename,
		         sizeof(filename),
		         "%s_%010d_%02d%02d%02d_%02d%02d%02d.jpg",
		         currentRequest->getGameServer()->getName().c_str(),
		         currentRequest->getGuildId(),
		         timeinfo.tm_year % 100,
		         timeinfo.tm_mon,
		         timeinfo.tm_mday,
		         timeinfo.tm_hour,
		         timeinfo.tm_min,
		         timeinfo.tm_sec);

		log(LL_Debug,
		    "Uploading image %s for client id %u with account id %u for guild %u\n",
		    filename,
		    currentRequest->getClientId(),
		    currentRequest->getAccountId(),
		    currentRequest->getGuildId());

		std::string absoluteDir = CONFIG_GET()->upload.client.uploadDir.get();
		std::string fullFileName = absoluteDir + "/" + filename;

		Utils::mkdir(absoluteDir.c_str());
		FILE* file = fopen(fullFileName.c_str(), "wb");
		if(!file) {
			log(LL_Warning, "Cant open upload target file %s\n", fullFileName.c_str());
			result.result = TS_RESULT_ACCESS_DENIED;
		} else {
			size_t dataWritten = fwrite(&packet->file_contents, packet->file_length, 1, file);
			fclose(file);

			if(dataWritten == 1) {
				result.result = TS_RESULT_SUCCESS;
				currentRequest->getGameServer()->sendUploadResult(
				    currentRequest->getGuildId(), packet->file_length, filename);
			} else {
				log(LL_Warning, "Cant write upload target file %s, disk full ?\n", fullFileName.c_str());
				result.result = TS_RESULT_ACCESS_DENIED;
			}
		}

		delete currentRequest;
		currentRequest = nullptr;
	}

	sendPacket(&result);
}

void ClientSession::onDownload(const TS_CU_DOWNLOAD_ICON* packet) {
	StreamAddress remoteAddress = getStream()->getRemoteAddress();
	char ip[INET6_ADDRSTRLEN];

	remoteAddress.getName(ip, sizeof(ip));

	log(LL_Debug, "Download from client %s:%d\n", ip, remoteAddress.port);

	std::string fileName = Utils::convertToString(packet->file_name, sizeof(packet->file_name) - 1);

	if(!IconServerSession::checkName(fileName.c_str(), fileName.size())) {
		// send error, invalid file name
		log(LL_Warning, "Download attempt of invalid filename: \"%s\"\n", fileName.c_str());
		return;
	}

	std::string fullFileName = CONFIG_GET()->upload.client.uploadDir.get() + "/" + fileName;
	std::unique_ptr<FILE, int (*)(FILE*)> file(fopen(fullFileName.c_str(), "rb"), fclose);

	if(!file) {
		// send error, file does not exists
		log(LL_Warning, "Download attempt of %s, but the file doesn\'t exist\n", fullFileName.c_str());
		return;
	}

	fseek(file.get(), 0, SEEK_END);
	size_t fileSize = ftell(file.get());
	fseek(file.get(), 0, SEEK_SET);

	if(fileSize > 64000) {
		// send error, file is too big
		log(LL_Warning,
		    "Download attempt of %s but file is too large (%" PRIuS " > 64000)\n",
		    fileName.c_str(),
		    fileSize);
		return;
	}

	std::unique_ptr<char[]> buffer(new char[fileSize]);

	size_t bytesTransferred = 0;
	size_t nbrw = 0;
	while(bytesTransferred < fileSize) {
		nbrw = fread(buffer.get() + bytesTransferred, 1, fileSize - bytesTransferred, file.get());
		if(nbrw <= 0)
			break;
		bytesTransferred += nbrw;
	}

	file.reset();

	if(nbrw <= 0) {
		// send error, can't read file
		log(LL_Warning, "Download attempt of %s but can\'t read file: %d\n", fileName.c_str(), errno);
		return;
	}

	// send file
	TS_UC_DOWNLOAD_ICON* result;
	result = TS_MESSAGE_WNA::create<TS_UC_DOWNLOAD_ICON, char>(fileSize);
	result->guild_id = packet->guild_id;
	result->icon_size = (uint32_t) fileSize;
	memcpy(result->guild_name, packet->guild_name, sizeof(packet->guild_name));
	memcpy(result->file_name, packet->file_name, sizeof(packet->file_name));
	memcpy(result->icon_data, buffer.get(), fileSize);

	buffer.reset();

	sendPacket(result);
}

bool ClientSession::checkJpegImage(uint32_t length, const unsigned char* data) {
	if(length < 2 || data[0] != 0xFF || data[1] != 0xD8)
		return false;

	return true;
}

}  // namespace UploadServer
