#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include "UploadRequest.h"
#include "IconServerSession.h"
#include <string.h>
#include "Utils.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_US_LOGIN_RESULT.h"
#include "Packets/TS_US_REQUEST_UPLOAD.h"
#include "Packets/TS_US_UPLOAD.h"

namespace UploadServer {

std::unordered_map<std::string, GameServerSession*>  GameServerSession::servers;

GameServerSession::GameServerSession() {
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

		default:
			debug("Unknown packet ID: %d, size: %d\n", packet->id, packet->size);
			break;
	}
}

void GameServerSession::onLogin(const TS_SU_LOGIN* packet) {
	TS_US_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_US_LOGIN_RESULT>(&result);
	typedef std::unordered_map<std::string, GameServerSession*>::iterator ServerIterator;

	std::string serverName = Utils::convertToString(packet->server_name, sizeof(packet->server_name)-1);

	info("Server Login: %s from %s:%d\n", serverName.c_str(), getStream()->getRemoteIpStr(), getStream()->getRemotePort());

	if(!IconServerSession::checkName(serverName.c_str(), serverName.size())) {
		//Forbidden character used in servername
		error("Server name (app.name in gameserver.opt) has invalid characters, only these are allowed: %s\n", IconServerSession::getAllowedCharsForName());
		result.result = TS_RESULT_INVALID_TEXT;
	} else {
		std::pair<ServerIterator, bool> insertResult = servers.insert(std::pair<std::string, GameServerSession*>(serverName, this));

		if(insertResult.second) {
			this->serverName = serverName;

			result.result = TS_RESULT_SUCCESS;
			setDirtyObjectName();
			debug("Success\n");
		} else {
			result.result = TS_RESULT_ALREADY_EXIST;
			error("Failed, server \"%s\" already registered\n", serverName.c_str());
		}
	}

	sendPacket(&result);
}

void GameServerSession::updateObjectName() {
	setObjectName(19 + serverName.size(), "GameServerSession[%s]", serverName.c_str());
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
