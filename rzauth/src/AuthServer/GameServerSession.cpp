#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include "ClientSession.h"
#include <string.h>
#include <algorithm>
#include "../GlobalConfig.h"
#include "PrintfFormats.h"
#include "LogServerClient.h"
#include <time.h>
#include "GameData.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/TS_AG_KICK_CLIENT.h"
#include "Packets/TS_AG_ITEM_PURCHASED.h"

namespace AuthServer {

GameServerSession::GameServerSession()
  : gameData(nullptr),
	useAutoReconnectFeature(false)
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
	if(gameData && useAutoReconnectFeature) {
		warn("Game server disconnected without logout\n");
		gameData->setGameServer(nullptr);
	} else if(gameData) {
		info("Server %d Logout\n", gameData->getServerIdx());
		GameData::remove(gameData);
	}
}

void GameServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_GA_LOGIN::packetIDEx:
			useAutoReconnectFeature = true;
			onServerLogin(static_cast<const TS_GA_LOGIN*>(packet));
			break;

		case TS_GA_LOGIN::packetID:
			useAutoReconnectFeature = false;
			onServerLogin(static_cast<const TS_GA_LOGIN*>(packet));
			break;

		case TS_GA_LOGOUT::packetID:
			onServerLogout(static_cast<const TS_GA_LOGOUT*>(packet));
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

	if(this->gameData) {
		result.result = TS_RESULT_INVALID_ARGUMENT;
		sendPacket(&result);
		error("Game server %s already sent login message\n", this->gameData->getServerName().c_str());
		return;
	}

	GameData* oldGameData = nullptr;
	GameData* gameData = GameData::tryAdd(this, packet->server_idx, localServerName, localServerIp, packet->server_port, localScreenshotUrl, packet->is_adult_server, &oldGameData);
	if(gameData) {
		result.result = TS_RESULT_SUCCESS;
		debug("Success\n");
		this->gameData = gameData;
	} else if(useAutoReconnectFeature && oldGameData && !oldGameData->getGameServer()) {
		if(oldGameData->getServerIdx() == packet->server_idx &&
				oldGameData->getServerName() == localServerName &&
				oldGameData->getServerIp() == localServerIp &&
				oldGameData->getServerPort() == packet->server_port &&
				oldGameData->getServerScreenshotUrl() == localScreenshotUrl &&
				oldGameData->getIsAdultServer() == packet->is_adult_server)
		{
			oldGameData->setGameServer(this);
			this->gameData = oldGameData;
		} else {
			result.result = TS_RESULT_ACCESS_DENIED;
			error("Failed, server index already used by another currently disconnected game server\n");
		}
	} else {
		result.result = TS_RESULT_ALREADY_EXIST;
		error("Failed, server index already used\n");
	}

	sendPacket(&result);
}

void GameServerSession::onServerLogout(const TS_GA_LOGOUT* packet) {
	if(gameData) {
		info("Server %d Logout\n", gameData->getServerIdx());
		GameData::remove(gameData);
		gameData = nullptr;
	} else {
		error("Received logout but game server was not logged in\n");
	}
}

void GameServerSession::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	TS_AG_CLIENT_LOGIN result;
	ClientData* client;
	char ipStr[INET_ADDRSTRLEN] = "";

	client = ClientData::getClient(std::string(packet->account));
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

	if(!gameData) {
		error("Received client login for account %s but game server is not logged on\n", result.account);
	} else if(client == nullptr || client->getGameServer() == nullptr) {
		warn("Client %s login on gameserver but not in client list\n", result.account);
	} else if(client->getGameServer() != gameData) {
		warn("Client %s login on wrong gameserver %s, expected %s\n", result.account, gameData->getServerName().c_str(), client->getGameServer() ? client->getGameServer()->getServerName().c_str() : "none");
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

		LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGIN, client->accountId, client->pcBang, client->eventCode, gameData->getServerIdx(), client->age, 0, 0, 0, 0, 0, 0,
				client->account.c_str(), -1, ipStr, -1, 0, 0, 0, 0);
	}

	sendPacket(&result);
}

void GameServerSession::onClientLogout(const TS_GA_CLIENT_LOGOUT* packet) {
	if(!gameData) {
		error("Received client logout for account %s but game server is not logged on\n", packet->account);
		return;
	}

	ClientData* clientData = ClientData::getClient(packet->account);

	debug("Client %s has been disconnected from gameserver%s\n", packet->account, clientData ? "" : " (not a known client)");

	if(!clientData)
		return;

	char ipStr[INET_ADDRSTRLEN];
	uv_inet_ntop(AF_INET, &clientData->ip, ipStr, sizeof(ipStr));

	LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGOUT, clientData->accountId, 0, 0, gameData->getServerIdx(), clientData->pcBang, 0, 0, 0, 0, 0, time(nullptr) - clientData->loginTime,
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
	if(!gameData) {
		error("Received client kick failed for account %s but game server is not logged on\n", packet->account);
		return;
	}

	std::string account(packet->account, std::find(packet->account, packet->account + sizeof(packet->account), '\0'));
	ClientData* clientData = ClientData::getClient(packet->account);

	warn("Client %s kick failed%s\n", account.c_str(), clientData ? " (removing from client list)" : "");

	if(!clientData)
		return;

	LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGOUT, clientData->accountId, 0, 0, gameData->getServerIdx(), clientData->pcBang, 0, 0, 0, 0, 0, time(nullptr) - clientData->loginTime,
			clientData->account.c_str(), -1, 0, 0, 0, 0, "FKICK", -1);

	ClientData::removeClient(clientData);
}

} // namespace AuthServer
