#define __STDC_LIMIT_MACROS
#include "GameServerInfo.h"
#include <string.h>
#include "../GlobalConfig.h"
#include "UploadRequest.h"

#include "EventLoop.h"
#include "Packets/PacketEnums.h"
#include "Packets/TS_US_LOGIN_RESULT.h"
#include "Packets/TS_US_REQUEST_UPLOAD.h"
#include "Packets/TS_US_UPLOAD.h"

namespace UploadServer {

std::unordered_map<std::string, GameServerInfo*>  GameServerInfo::servers;

GameServerInfo::GameServerInfo(RappelzSocket* socket) {
	this->socket = socket;

	socket->addEventListener(this, &onStateChanged);
	socket->addPacketListener(TS_SU_LOGIN::packetID, this, &onDataReceived);
	socket->addPacketListener(TS_SU_REQUEST_UPLOAD::packetID, this, &onDataReceived);
}

void GameServerInfo::startServer() {
	Socket* serverSocket = new Socket(EventLoop::getLoop());
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen(CONFIG_GET()->upload.game.listenIp,
						 CONFIG_GET()->upload.game.port);
}

GameServerInfo::~GameServerInfo() {
	if(this->serverName.empty() == false)
		servers.erase(this->serverName);
	socket->deleteLater();
}

void GameServerInfo::onNewConnection(ICallbackGuard* instance, Socket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket(EventLoop::getLoop(), false);
	static GameServerInfo* serverInfo = new GameServerInfo(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		newSocket = new RappelzSocket(EventLoop::getLoop(), false);
		serverInfo = new GameServerInfo(newSocket);
	} while(1);
}

void GameServerInfo::onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	GameServerInfo* thisInstance = static_cast<GameServerInfo*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void GameServerInfo::onDataReceived(ICallbackGuard* instance, RappelzSocket* , const TS_MESSAGE* packet) {
	GameServerInfo* thisInstance = static_cast<GameServerInfo*>(instance);

	switch(packet->id) {
		case TS_SU_LOGIN::packetID:
			thisInstance->onLogin(static_cast<const TS_SU_LOGIN*>(packet));
			break;

		case TS_SU_REQUEST_UPLOAD::packetID:
			thisInstance->onRequestUpload(static_cast<const TS_SU_REQUEST_UPLOAD*>(packet));
			break;
	}
}

void GameServerInfo::onLogin(const TS_SU_LOGIN* packet) {
	TS_US_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_US_LOGIN_RESULT>(&result);
	typedef std::unordered_map<std::string, GameServerInfo*>::iterator ServerIterator;

	info("Server Login: %s from %s:%d\n", packet->server_name, socket->getHost().c_str(), socket->getPort());

	std::pair<ServerIterator, bool> insertResult = servers.insert(std::pair<std::string, GameServerInfo*>(packet->server_name, this));

	if(insertResult.second) {
		serverName = packet->server_name;

		result.result = TS_RESULT_SUCCESS;
		setObjectName(16 + serverName.size(), "GameServerInfo[%s]", serverName.c_str());
		debug("Success\n");
	} else {
		result.result = TS_RESULT_ALREADY_EXIST;
		error("Failed, server \"%s\" already registered\n", packet->server_name);
	}

	socket->sendPacket(&result);
}

void GameServerInfo::onRequestUpload(const TS_SU_REQUEST_UPLOAD* packet) {
	TS_US_REQUEST_UPLOAD result;
	TS_MESSAGE::initMessage<TS_US_REQUEST_UPLOAD>(&result);

	debug("Upload request for client %u with account id %u for guild %u\n", packet->client_id, packet->account_id, packet->guild_sid);

	UploadRequest::pushRequest(this, packet->client_id, packet->account_id, packet->guild_sid, packet->one_time_password);

	result.result = TS_RESULT_SUCCESS;
	socket->sendPacket(&result);
}

void GameServerInfo::sendUploadResult(uint32_t guidId, uint32_t fileSize, const char* fileName) {
	int fileNameSize = strlen(fileName);
	TS_US_UPLOAD *result = TS_MESSAGE_WNA::create<TS_US_UPLOAD, char>(fileNameSize);

	result->guild_id = guidId;
	result->file_size = fileSize;
	result->filename_length = strlen(fileName);
	result->type = 0;
	strncpy(result->file_name, fileName, fileNameSize);
	socket->sendPacket(result);

	TS_MESSAGE_WNA::destroy(result);
}

} //namespace UploadServer
