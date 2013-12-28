#define __STDC_LIMIT_MACROS
#include "ServerInfo.h"
#include "ClientInfo.h"
#include <string.h>
#include "GlobalConfig.h"
#include <time.h>

#include "EventLoop.h"
#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/TS_AG_KICK_CLIENT.h"

std::vector<ServerInfo*> ServerInfo::servers;

ServerInfo::ServerInfo(RappelzSocket* socket)
{
	this->socket = socket;
	serverIdx = UINT16_MAX;

	addInstance(socket->addEventListener(this, &onStateChanged));
	addInstance(socket->addPacketListener(TS_GA_LOGIN::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_GA_CLIENT_LOGIN::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_GA_CLIENT_LOGOUT::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_GA_CLIENT_KICK_FAILED::packetID, this, &onDataReceived));
}

void ServerInfo::startServer() {
	Socket* serverSocket = new Socket(EventLoop::getLoop());
	srand((unsigned int)time(NULL));
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen(CONFIG_GET()->gameConfig.listenIp,
						 CONFIG_GET()->gameConfig.port);
}

ServerInfo::~ServerInfo() {
	invalidateCallbacks();

	if(serverIdx != UINT16_MAX && (size_t)serverIdx < servers.size())
		servers[serverIdx] = nullptr;

	socket->deleteLater();
}

void ServerInfo::onNewConnection(void* instance, Socket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket(EventLoop::getLoop(), false);
	static ServerInfo* serverInfo = new ServerInfo(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		newSocket = new RappelzSocket(EventLoop::getLoop(), false);
		serverInfo = new ServerInfo(newSocket);
	} while(1);
}

void ServerInfo::onStateChanged(void* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	ServerInfo* thisInstance = static_cast<ServerInfo*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void ServerInfo::onDataReceived(void* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet) {
	ServerInfo* thisInstance = static_cast<ServerInfo*>(instance);

	switch(packet->id) {
		case TS_GA_LOGIN::packetID:
			thisInstance->onServerLogin(static_cast<const TS_GA_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGIN::packetID:
			thisInstance->onClientLogin(static_cast<const TS_GA_CLIENT_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGOUT::packetID:
			thisInstance->onClientLogout(static_cast<const TS_GA_CLIENT_LOGOUT*>(packet));
			break;

		case TS_GA_CLIENT_KICK_FAILED::packetID:
			thisInstance->onClientKickFailed(static_cast<const TS_GA_CLIENT_KICK_FAILED*>(packet));
			break;
	}
}

void ServerInfo::onServerLogin(const TS_GA_LOGIN* packet) {
	TS_AG_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_AG_LOGIN_RESULT>(&result);

	log("Server Login: %s[%d] at %s:%d\n", packet->server_name, packet->server_idx, packet->server_ip, packet->server_port);

	if(packet->server_idx == UINT16_MAX) {
		result.result = TS_RESULT_INVALID_ARGUMENT;
		log("GameServer attempt to use reserved id, change to a lower id\n");
		socket->sendPacket(&result);
		return;
	}

	if(servers.size() <= (size_t)packet->server_idx || servers.at(packet->server_idx) == nullptr) {
		serverIdx = packet->server_idx;
		serverName = packet->server_name;
		serverIp = packet->server_ip;
		serverPort = packet->server_port;
		serverScreenshotUrl = packet->server_screenshot_url;
		isAdultServer = packet->is_adult_server;

		if(servers.size() <= (size_t)serverIdx)
			servers.resize(serverIdx+1, nullptr);
		servers[serverIdx] = this;

		result.result = TS_RESULT_SUCCESS;
		setObjectName((std::string(getObjectName()) + "[" + packet->server_name + "]").c_str());
		log("Success\n");
	} else {
		result.result = TS_RESULT_ALREADY_EXIST;
		log("Failed, server index already used\n");
	}

	socket->sendPacket(&result);
}

void ServerInfo::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	TS_AG_CLIENT_LOGIN result;
	ClientData* client = ClientData::getClient(std::string(packet->account));

	TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN>(&result);
	strncpy(result.account, packet->account, 61);

	result.nAccountID = 0;
	result.result = TS_RESULT_ACCESS_DENIED;
	result.nPCBangUser = 0;
	result.nEventCode = 0;
	result.nAge = 0;
	result.nContinuousPlayTime = 0;
	result.nContinuousLogoutTime = 0;

	if(client == nullptr) {
		log("Client %s login on gameserver but not in clientData list\n", packet->account);
	} else if(client->server != this) {
		log("Client %s login on wrong gameserver %s, expected %s\n", packet->account, serverName.c_str(), client->server->serverName.c_str());
	} else if(client->oneTimePassword != packet->one_time_key) {
		log("Client %s login on gameserver but wrong one time password: expected %lu but received %lu\n", packet->account, client->oneTimePassword, packet->one_time_key);
	} else if(client->inGame) {
		log("Client %s login on gameserver but already connected\n", packet->account);
	} else {
		//To complete
		log("Client %s now on gameserver\n", packet->account);
		result.nAccountID = client->accountId;
		result.result = TS_RESULT_SUCCESS;
		result.nPCBangUser = 0;
		result.nEventCode = client->eventCode;
		result.nAge = client->age;
		result.nContinuousPlayTime = 0;
		result.nContinuousLogoutTime = 0;

		client->inGame = true;
	}

	socket->sendPacket(&result);
}

void ServerInfo::onClientLogout(const TS_GA_CLIENT_LOGOUT* packet) {
	log("Client %s has disconnected from gameserver\n", packet->account);
	ClientData::removeClient(packet->account);
}

void ServerInfo::kickClient(const std::string &account) {
	TS_AG_KICK_CLIENT msg;

	TS_MESSAGE::initMessage<TS_AG_KICK_CLIENT>(&msg);
	strcpy(msg.account, account.c_str());
	msg.kick_type = TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN;

	socket->sendPacket(&msg);
}

void ServerInfo::onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet) {
	log("Client %s kick failed (removing from client list)\n", packet->account);
	ClientData::removeClient(packet->account);
}
