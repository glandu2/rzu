#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include "ClientSession.h"
#include <string.h>
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
	if(gameData && gameData->getGameServer() == this) {
		if(useAutoReconnectFeature) {
			warn("Game server disconnected without logout\n");
			setGameData(nullptr);
		} else {
			info("Server %d Logout\n", gameData->getServerIdx());
			GameData::remove(gameData);
		}
	}
	// else the server connection changed (this one was maybe halfopen)
}

void GameServerSession::onConnected() {
	getStream()->setKeepAlive(30);
}

void GameServerSession::setGameData(GameData* gameData) {
	GameData* oldGameData = this->gameData;

	if(this->gameData != gameData) {
		this->gameData = gameData;

		if(oldGameData)
			oldGameData->setGameServer(nullptr);
		if(gameData) {
			gameData->setGameServer(this);
			debug("Set game data to %s\n", gameData->getObjectName());
		}
	}
}

void GameServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_GA_LOGIN_WITH_LOGOUT::packetID:
			useAutoReconnectFeature = true;
			onServerLogin(static_cast<const TS_GA_LOGIN_WITH_LOGOUT*>(packet));
			break;

		case TS_GA_LOGIN::packetID:
			useAutoReconnectFeature = false;
			onServerLogin(static_cast<const TS_GA_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGGED_LIST::packetID:
			onAccountList(static_cast<const TS_GA_CLIENT_LOGGED_LIST*>(packet));
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

	localServerName = Utils::convertToString(packet->server_name, sizeof(packet->server_name));
	localServerIp = Utils::convertToString(packet->server_ip, sizeof(packet->server_ip));
	localScreenshotUrl = Utils::convertToString(packet->server_screenshot_url, sizeof(packet->server_screenshot_url));

	info("Server Login: %s[%d] at %s:%d\n", localServerName.c_str(), packet->server_idx, localServerIp.c_str(), packet->server_port);

	if(this->gameData) {
		result.result = TS_RESULT_INVALID_ARGUMENT;
		sendPacket(&result);
		error("Game server %s already sent login message\n", this->gameData->getServerName().c_str());
		return;
	}

	result.result = TS_RESULT_UNKNOWN;

	GameData* oldGameData = nullptr;
	GameData* gameData = GameData::tryAdd(this, packet->server_idx, localServerName, localServerIp, packet->server_port, localScreenshotUrl, packet->is_adult_server, &oldGameData);
	if(gameData) {
		result.result = TS_RESULT_SUCCESS;
		debug("Success\n");
		setGameData(gameData);
		if(!useAutoReconnectFeature)
			gameData->setReady(true);
		else
			gameData->setReady(false);
	} else if(oldGameData) {
		if(useAutoReconnectFeature &&
				oldGameData->getServerIdx() == packet->server_idx &&
				oldGameData->getServerName() == localServerName &&
				oldGameData->getServerIp() == localServerIp &&
				oldGameData->getServerPort() == packet->server_port &&
				oldGameData->getServerScreenshotUrl() == localScreenshotUrl &&
				oldGameData->getIsAdultServer() == packet->is_adult_server)
		{
			GameServerSession* gameServer = oldGameData->getGameServer();

			result.result = TS_RESULT_SUCCESS;
			setGameData(oldGameData);
			oldGameData->setReady(false);

			if(gameServer) {
				info("Received same game info for server %s[%d] but from different connection, dropping the old one (maybe halfopen ?)\n",
					 localServerName.c_str(), packet->server_idx);
				gameServer->closeSession();
			} else {
				info("Game server %s[%d] reconnected\n", localServerName.c_str(), packet->server_idx);
			}
		} else {
			result.result = TS_RESULT_ALREADY_EXIST;
			error("Failed, server index already used\n");
		}
	}

	sendPacket(&result);
}

void GameServerSession::onAccountList(const TS_GA_CLIENT_LOGGED_LIST *packet) {
	if(!gameData) {
		error("Received account list but game server is not logged on\n");
		return;
	}

	for(uint8_t i = 0; i < packet->count; i++) {
		alreadyConnectedAccounts.push_back(packet->accountInfo[i]);
	}
	debug("Added %d accounts\n", packet->count);

	if(packet->final_packet) {
		//Remove all account connected on this server (they will be recreated after that using the list)
		ClientData::removeServer(gameData);

		for(size_t i = 0; i < alreadyConnectedAccounts.size(); i++) {
			const TS_GA_CLIENT_LOGGED_LIST::AccountInfo& accountInfo = alreadyConnectedAccounts[i];

			debug("Adding already connected account %s\n", accountInfo.account);

			ClientData* clientData = ClientData::tryAddClient(nullptr,
															  accountInfo.account,
															  accountInfo.nAccountID,
															  accountInfo.nAge,
															  accountInfo.nEventCode,
															  accountInfo.nPCBangUser,
															  accountInfo.ip);
			if(clientData) {
				clientData->switchClientToServer(gameData, 0);
				clientData->connectedToGame();
				clientData->loginTime = accountInfo.loginTime;
			} else {
				//Client is connected somewhere else
				TS_AG_KICK_CLIENT msg;

				TS_MESSAGE::initMessage<TS_AG_KICK_CLIENT>(&msg);
				strcpy(msg.account, accountInfo.account);
				msg.kick_type = TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN;

				info("Kicked client %s while synchronizing account list\n", accountInfo.account);
				sendPacket(&msg);
			}
		}
		alreadyConnectedAccounts.clear();
		gameData->setReady(true);
	}
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
	ClientData* client;
	char account[61];

	client = ClientData::getClient(std::string(packet->account));

	memcpy(account, packet->account, sizeof(account));
	account[sizeof(account) - 1] = '\0';

	TS_ResultCode result = TS_RESULT_ACCESS_DENIED;

	if(!gameData) {
		error("Received client login for account %s but game server is not logged on\n", account);
	} else if(client == nullptr || client->getGameServer() == nullptr) {
		warn("Client %s login on gameserver but not in client list\n", account);
	} else if(client->getGameServer() != gameData) {
		warn("Client %s login on wrong gameserver %s, expected %s\n", account, gameData->getServerName().c_str(), client->getGameServer() ? client->getGameServer()->getServerName().c_str() : "none");
	} else if(gameData->isReady() == false) {
		warn("Client %s login on a not ready gameserver\n", account);
	} else if(client->oneTimePassword != packet->one_time_key) {
		warn("Client %s login on gameserver but wrong one time password: expected %" PRIu64 " but received %" PRIu64 "\n", account, client->oneTimePassword, packet->one_time_key);
	} else if(client->isConnectedToGame()) {
		info("Client %s login on gameserver but already connected\n", account);
	} else {
		//To complete
		debug("Client %s now on gameserver\n", account);
		result = TS_RESULT_SUCCESS;

		client->connectedToGame();

		char ipStr[INET_ADDRSTRLEN] = "";
		uv_inet_ntop(AF_INET, &client->ip, ipStr, sizeof(ipStr));

		LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGIN, client->accountId, client->pcBang, client->eventCode, gameData->getServerIdx(), client->age, 0, 0, 0, 0, 0, 0,
				client->account.c_str(), -1, ipStr, -1, 0, 0, 0, 0);
	}

	sendClientLoginResult(account, result, client);
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

	std::string account = Utils::convertToString(packet->account, sizeof(packet->account));
	ClientData* clientData = ClientData::getClient(packet->account);

	warn("Client %s kick failed%s\n", account.c_str(), clientData ? " (removing from client list)" : "");

	if(!clientData)
		return;

	LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGOUT, clientData->accountId, 0, 0, gameData->getServerIdx(), clientData->pcBang, 0, 0, 0, 0, 0, time(nullptr) - clientData->loginTime,
			clientData->account.c_str(), -1, 0, 0, 0, 0, "FKICK", -1);

	ClientData::removeClient(clientData);
}

void GameServerSession::sendClientLoginResult(const char* account, TS_ResultCode result, ClientData* clientData) {
	if(useAutoReconnectFeature) {
		TS_AG_CLIENT_LOGIN_EXTENDED packet;
		TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN_EXTENDED>(&packet);
		fillClientLoginExtendedResult(&packet, account, result, clientData);

		sendPacket(&packet);
	} else {
		TS_AG_CLIENT_LOGIN packet;
		TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN>(&packet);
		fillClientLoginResult(&packet, account, result, clientData);

		sendPacket(&packet);
	}
}

void GameServerSession::fillClientLoginResult(TS_AG_CLIENT_LOGIN* packet, const char* account, TS_ResultCode result, ClientData* clientData) {
	strcpy(packet->account, account);
	packet->result = result;
	if(result == TS_RESULT_SUCCESS && clientData) {
		packet->nAccountID = clientData->accountId;
		packet->nPCBangUser = clientData->pcBang != 0;
		packet->nEventCode = clientData->eventCode;
		packet->nAge = clientData->age;
		packet->nContinuousPlayTime = 0;
		packet->nContinuousLogoutTime = 0;
	} else {
		packet->nAccountID = 0;
		packet->nPCBangUser = 0;
		packet->nEventCode = 0;
		packet->nAge = 0;
		packet->nContinuousPlayTime = 0;
		packet->nContinuousLogoutTime = 0;
	}
}

void GameServerSession::fillClientLoginExtendedResult(TS_AG_CLIENT_LOGIN_EXTENDED* packet, const char* account, TS_ResultCode result, ClientData* clientData) {
	fillClientLoginResult(packet, account, result, clientData);
	if(result == TS_RESULT_SUCCESS && clientData) {
		packet->ip = clientData->ip;
		packet->loginTime = (uint32_t) clientData->loginTime;
	} else {
		packet->ip = 0;
		packet->loginTime = 0;
	}
}

} // namespace AuthServer
