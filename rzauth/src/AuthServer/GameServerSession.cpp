#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include "ClientSession.h"
#include <string.h>
#include <algorithm>

#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/TS_AG_KICK_CLIENT.h"

namespace AuthServer {

std::vector<GameServerSession*> GameServerSession::servers;

GameServerSession::GameServerSession() : RappelzSession(EncryptedSocket::NoEncryption) {
	addPacketsToListen(4,
					   TS_GA_LOGIN::packetID,
					   TS_GA_CLIENT_LOGIN::packetID,
					   TS_GA_CLIENT_LOGOUT::packetID,
					   TS_GA_CLIENT_KICK_FAILED::packetID
					   );
	serverIdx = UINT16_MAX;
}

GameServerSession::~GameServerSession() {
	if(serverIdx != UINT16_MAX && (size_t)serverIdx < servers.size()) {
		servers[serverIdx] = nullptr;
		info("Server %d Logout\n", serverIdx);
		ClientData::removeServer(this);
	}
}

void GameServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_GA_LOGIN::packetID:
			onServerLogin(static_cast<const TS_GA_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGIN::packetID:
			onClientLogin(static_cast<const TS_GA_CLIENT_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGOUT::packetID:
			onClientLogout(static_cast<const TS_GA_CLIENT_LOGOUT*>(packet));
			break;

		case TS_GA_CLIENT_KICK_FAILED::packetID:
			onClientKickFailed(static_cast<const TS_GA_CLIENT_KICK_FAILED*>(packet));
			break;
	}
}

void GameServerSession::onServerLogin(const TS_GA_LOGIN* packet) {
	TS_AG_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_AG_LOGIN_RESULT>(&result);

	//to manage non null terminated strings
	std::string localServerName, localServerIp, localScreenshotUrl;

	localServerName = std::string(packet->server_name, std::find(packet->server_name, packet->server_name + sizeof(packet->server_name), '\0'));
	localServerIp = std::string(packet->server_ip, std::find(packet->server_ip, packet->server_ip + sizeof(packet->server_ip), '\0'));
	localScreenshotUrl = std::string(packet->server_screenshot_url, std::find(packet->server_screenshot_url, packet->server_screenshot_url + sizeof(packet->server_screenshot_url), '\0'));

	info("Server Login: %s[%d] at %s:%d\n", localServerName.c_str(), packet->server_idx, localServerIp.c_str(), packet->server_port);

	if(packet->server_idx == UINT16_MAX) {
		result.result = TS_RESULT_INVALID_ARGUMENT;
		error("GameServer attempt to use reserved id, change to a lower id\n");
		sendPacket(&result);
		return;
	}

	if(servers.size() <= (size_t)packet->server_idx || servers.at(packet->server_idx) == nullptr) {
		serverIdx = packet->server_idx;
		serverName.swap(localServerName);
		serverIp.swap(localServerIp);
		serverPort = packet->server_port;
		serverScreenshotUrl.swap(localScreenshotUrl);
		isAdultServer = packet->is_adult_server;

		if(servers.size() <= (size_t)serverIdx)
			servers.resize(serverIdx+1, nullptr);
		servers[serverIdx] = this;

		result.result = TS_RESULT_SUCCESS;
		setObjectName(12 + serverName.size(), "ServerInfo[%s]", serverName.c_str());
		debug("Success\n");
	} else {
		result.result = TS_RESULT_ALREADY_EXIST;
		error("Failed, server index already used\n");
	}

	sendPacket(&result);
}

void GameServerSession::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	TS_AG_CLIENT_LOGIN result;
	ClientData* client = ClientData::getClient(std::string(packet->account));

	TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN>(&result);
	memcpy(result.account, packet->account, sizeof(result.account));
	result.account[sizeof(result.account) - 1] = '\0';

	result.nAccountID = 0;
	result.result = TS_RESULT_ACCESS_DENIED;
	result.nPCBangUser = 0;
	result.nEventCode = 0;
	result.nAge = 0;
	result.nContinuousPlayTime = 0;
	result.nContinuousLogoutTime = 0;

	if(client == nullptr) {
		warn("Client %s login on gameserver but not in client list\n", result.account);
	} else if(client->server != this) {
		warn("Client %s login on wrong gameserver %s, expected %s\n", result.account, serverName.c_str(), client->server->serverName.c_str());
	} else if(client->oneTimePassword != packet->one_time_key) {
		warn("Client %s login on gameserver but wrong one time password: expected %lu but received %lu\n", result.account, client->oneTimePassword, packet->one_time_key);
	} else if(client->inGame) {
		info("Client %s login on gameserver but already connected\n", result.account);
	} else {
		//To complete
		trace("Client %s now on gameserver\n", result.account);
		result.nAccountID = client->accountId;
		result.result = TS_RESULT_SUCCESS;
		result.nPCBangUser = 0;
		result.nEventCode = client->eventCode;
		result.nAge = client->age;
		result.nContinuousPlayTime = 0;
		result.nContinuousLogoutTime = 0;

		client->inGame = true;
	}

	sendPacket(&result);
}

void GameServerSession::onClientLogout(const TS_GA_CLIENT_LOGOUT* packet) {
	trace("Client %s has disconnected from gameserver\n", packet->account);
	ClientData::removeClient(packet->account);
}

void GameServerSession::kickClient(const std::string &account) {
	TS_AG_KICK_CLIENT msg;

	TS_MESSAGE::initMessage<TS_AG_KICK_CLIENT>(&msg);
	strcpy(msg.account, account.c_str());
	msg.kick_type = TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN;

	sendPacket(&msg);
}

void GameServerSession::onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet) {
	std::string account(packet->account, std::find(packet->account, packet->account + sizeof(packet->account), '\0'));

	warn("Client %s kick failed (removing from client list)\n", account.c_str());
	ClientData::removeClient(account);
}

} // namespace AuthServer
