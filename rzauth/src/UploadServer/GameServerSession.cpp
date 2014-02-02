#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include "UploadRequest.h"
#include <string.h>

#include "Packets/PacketEnums.h"
#include "Packets/TS_US_LOGIN_RESULT.h"
#include "Packets/TS_US_REQUEST_UPLOAD.h"
#include "Packets/TS_US_UPLOAD.h"

namespace UploadServer {

std::unordered_map<std::string, GameServerSession*>  GameServerSession::servers;

GameServerSession::GameServerSession() : RappelzSession(EncryptedSocket::NoEncryption) {
	addPacketsToListen(2,
					   TS_SU_LOGIN::packetID,
					   TS_SU_REQUEST_UPLOAD::packetID
					   );
}

GameServerSession::~GameServerSession() {
	if(this->serverName.empty() == false) {
		servers.erase(this->serverName);
		info("Server Logout\n");
	}
}

void GameServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_SU_LOGIN::packetID:
			onLogin(static_cast<const TS_SU_LOGIN*>(packet));
			break;

		case TS_SU_REQUEST_UPLOAD::packetID:
			onRequestUpload(static_cast<const TS_SU_REQUEST_UPLOAD*>(packet));
			break;
	}
}

void GameServerSession::onLogin(const TS_SU_LOGIN* packet) {
	TS_US_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_US_LOGIN_RESULT>(&result);
	typedef std::unordered_map<std::string, GameServerSession*>::iterator ServerIterator;

	info("Server Login: %s from %s:%d\n", packet->server_name, getSocket()->getHost().c_str(), getSocket()->getPort());

	std::pair<ServerIterator, bool> insertResult = servers.insert(std::pair<std::string, GameServerSession*>(packet->server_name, this));

	if(insertResult.second) {
		serverName = packet->server_name;

		result.result = TS_RESULT_SUCCESS;
		setObjectName(16 + serverName.size(), "GameServerInfo[%s]", serverName.c_str());
		debug("Success\n");
	} else {
		result.result = TS_RESULT_ALREADY_EXIST;
		error("Failed, server \"%s\" already registered\n", packet->server_name);
	}

	sendPacket(&result);
}

void GameServerSession::onRequestUpload(const TS_SU_REQUEST_UPLOAD* packet) {
	TS_US_REQUEST_UPLOAD result;
	TS_MESSAGE::initMessage<TS_US_REQUEST_UPLOAD>(&result);

	debug("Upload request for client %u with account id %u for guild %u\n", packet->client_id, packet->account_id, packet->guild_sid);

	UploadRequest::pushRequest(this, packet->client_id, packet->account_id, packet->guild_sid, packet->one_time_password);

	result.result = TS_RESULT_SUCCESS;
	sendPacket(&result);
}

void GameServerSession::sendUploadResult(uint32_t guidId, uint32_t fileSize, const char* fileName) {
	int fileNameSize = strlen(fileName);
	TS_US_UPLOAD *result = TS_MESSAGE_WNA::create<TS_US_UPLOAD, char>(fileNameSize);

	result->guild_id = guidId;
	result->file_size = fileSize;
	result->filename_length = strlen(fileName);
	result->type = 0;
	strncpy(result->file_name, fileName, fileNameSize);
	sendPacket(result);

	TS_MESSAGE_WNA::destroy(result);
}

} //namespace UploadServer
