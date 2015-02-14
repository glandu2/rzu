#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include "ClientSession.h"
#include <string.h>
#include <algorithm>
#include "../GlobalConfig.h"
#include "PrintfFormats.h"
#include "LogServerClient.h"
#include <time.h>

#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/TS_AG_KICK_CLIENT.h"
#include "Packets/TS_AG_ITEM_PURCHASED.h"

namespace AuthServer {

std::unordered_map<uint16_t, GameServerSession*> GameServerSession::servers;

GameServerSession::GameServerSession()
  : serverIdx(UINT16_MAX),
	serverPort(0),
	isAdultServer(false),
	playerCount(0)
{
}

void GameServerSession::sendNotifyItemPurchased(ClientData* client) {
	TS_AG_ITEM_PURCHASED itemPurchasedPacket;
	TS_MESSAGE::initMessage<TS_AG_ITEM_PURCHASED>(&itemPurchasedPacket);

	strncpy(itemPurchasedPacket.account, client->account.c_str(), 60);
	itemPurchasedPacket.account[60] = 0;

	itemPurchasedPacket.nAccountID = client->accountId;

	sendPacket(&itemPurchasedPacket);
}

GameServerSession::~GameServerSession() {
	if(serverIdx != UINT16_MAX) {
		servers.erase(serverIdx);
		info("Server %d Logout\n", serverIdx);
		LogServerClient::sendLog(LogServerClient::LM_GAME_SERVER_LOGOUT, 0, 0, 0, serverIdx, serverPort, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, serverIp.c_str(), -1, serverName.c_str(), -1);
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

		default:
			debug("Unknown packet ID: %d, size: %d\n", packet->id, packet->size);
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

	if(servers.find(packet->server_idx) == servers.end()) {
		serverIdx = packet->server_idx;
		serverName.swap(localServerName);
		serverIp.swap(localServerIp);
		serverPort = packet->server_port;
		serverScreenshotUrl.swap(localScreenshotUrl);
		isAdultServer = packet->is_adult_server;

		servers.insert(std::pair<uint16_t, GameServerSession*>(serverIdx, this));

		result.result = TS_RESULT_SUCCESS;
		setDirtyObjectName();
		debug("Success\n");

		LogServerClient::sendLog(LogServerClient::LM_GAME_SERVER_LOGIN, 0, 0, 0, serverIdx, serverPort, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, serverIp.c_str(), -1, serverName.c_str(), -1);
	} else {
		result.result = TS_RESULT_ALREADY_EXIST;
		error("Failed, server index already used\n");
	}

	sendPacket(&result);
}

void GameServerSession::updateObjectName() {
	setObjectName(12 + serverName.size(), "ServerInfo[%s]", serverName.c_str());
}

void GameServerSession::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	TS_AG_CLIENT_LOGIN result;
	ClientData* client = ClientData::getClient(std::string(packet->account));
	char ipStr[INET_ADDRSTRLEN] = "";

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

	if(client)
		uv_inet_ntop(AF_INET, &client->ip, ipStr, sizeof(ipStr));

	if(client == nullptr) {
		warn("Client %s login on gameserver but not in client list\n", result.account);
	} else if(client->getGameServer() != this) {
		warn("Client %s login on wrong gameserver %s, expected %s\n", result.account, serverName.c_str(), client->getGameServer() ? client->getGameServer()->serverName.c_str() : "none");
	} else if(client->oneTimePassword != packet->one_time_key) {
		warn("Client %s login on gameserver but wrong one time password: expected %" PRIu64 " but received %" PRIu64 "\n", result.account, client->oneTimePassword, packet->one_time_key);
	} else if(client->isConnectedToGame()) {
		info("Client %s login on gameserver but already connected\n", result.account);
	} else {
		//To complete
		debug("Client %s now on gameserver\n", result.account);
		result.nAccountID = client->accountId;
		result.result = TS_RESULT_SUCCESS;
		result.nPCBangUser = client->pcBang != 0;
		result.nEventCode = client->eventCode;
		result.nAge = client->age;
		result.nContinuousPlayTime = 0;
		result.nContinuousLogoutTime = 0;

		client->connectedToGame();

		LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGIN, client->accountId, client->pcBang, client->eventCode, serverIdx, client->age, 0, 0, 0, 0, 0, 0,
				client->account.c_str(), -1, ipStr, -1, 0, 0, 0, 0);
	}

	sendPacket(&result);
}

void GameServerSession::onClientLogout(const TS_GA_CLIENT_LOGOUT* packet) {
	ClientData* clientData = ClientData::getClient(packet->account);

	debug("Client %s has been disconnected from gameserver%s\n", packet->account, clientData ? "" : " (not a known client)");

	if(!clientData)
		return;

	char ipStr[INET_ADDRSTRLEN];
	uv_inet_ntop(AF_INET, &clientData->ip, ipStr, sizeof(ipStr));

	LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGOUT, clientData->accountId, 0, 0, serverIdx, clientData->pcBang, 0, 0, 0, 0, 0, time(nullptr) - clientData->loginTime,
			clientData->account.c_str(), -1, ipStr, -1, 0, 0, clientData->kickRequested ? "DKICK" : "NORMAL", -1);

	ClientData::removeClient(clientData);
}

void GameServerSession::kickClient(ClientData* clientData) {
	TS_AG_KICK_CLIENT msg;

	TS_MESSAGE::initMessage<TS_AG_KICK_CLIENT>(&msg);
	strcpy(msg.account, clientData->account.c_str());
	msg.kick_type = TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN;

	sendPacket(&msg);

	if(!CONFIG_GET()->auth.game.strictKick.get())
		ClientData::removeClient(clientData);
	else
		clientData->kickRequested = true;
}

void GameServerSession::onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet) {
	std::string account(packet->account, std::find(packet->account, packet->account + sizeof(packet->account), '\0'));
	ClientData* clientData = ClientData::getClient(packet->account);

	warn("Client %s kick failed%s\n", account.c_str(), clientData ? " (removing from client list)" : "");

	if(!clientData)
		return;

	LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGOUT, clientData->accountId, 0, 0, serverIdx, clientData->pcBang, 0, 0, 0, 0, 0, time(nullptr) - clientData->loginTime,
			clientData->account.c_str(), -1, 0, 0, 0, 0, "FKICK", -1);

	ClientData::removeClient(clientData);
}

} // namespace AuthServer
