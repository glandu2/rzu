#include "GameServerSession.h"
#include "../GlobalConfig.h"
#include "ClientSession.h"
#include "Core/PrintfFormats.h"
#include "GameData.h"
#include "LogServerClient.h"
#include <string.h>
#include <time.h>

#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "AuthGame/TS_AG_ITEM_PURCHASED.h"
#include "AuthGame/TS_AG_ITEM_SUPPLIED.h"
#include "AuthGame/TS_AG_KICK_CLIENT.h"
#include "AuthGame/TS_AG_LOGIN_RESULT.h"
#include "AuthGame/TS_AG_SECURITY_NO_CHECK.h"
#include "PacketEnums.h"

namespace AuthServer {

GameServerSession::GameServerSession()
    : PacketSession(SessionType::AuthGame, SessionPacketOrigin::Server, EPIC_LATEST),
      gameData(nullptr),
      useAutoReconnectFeature(false),
      securityNoSendMode(true) {}

void GameServerSession::sendNotifyItemPurchased(ClientData* client) {
	TS_AG_ITEM_PURCHASED itemPurchasedPacket;
	TS_MESSAGE::initMessage<TS_AG_ITEM_PURCHASED>(&itemPurchasedPacket);

	strncpy(itemPurchasedPacket.account, client->account.c_str(), 60);
	itemPurchasedPacket.account[60] = 0;

	itemPurchasedPacket.nAccountID = client->accountId;

	sendPacket(&itemPurchasedPacket);
}

void GameServerSession::sendNotifyItemSupplied(ClientData* client) {
	TS_AG_ITEM_SUPPLIED itemSuppliedPacket;
	TS_MESSAGE::initMessage<TS_AG_ITEM_SUPPLIED>(&itemSuppliedPacket);

	strncpy(itemSuppliedPacket.account, client->account.c_str(), 60);
	itemSuppliedPacket.account[60] = 0;

	sendPacket(&itemSuppliedPacket);
}

GameServerSession::~GameServerSession() {
	if(gameData && gameData->getGameServer() == this) {
		if(useAutoReconnectFeature) {
			log(LL_Warning, "Game server disconnected without logout\n");
			setGameData(nullptr);
		} else {
			log(LL_Info, "Server %d Logout\n", gameData->getServerIdx());
			GameData::remove(gameData);
		}
	}
	// else the server connection changed (this one was maybe halfopen)
}

EventChain<SocketSession> GameServerSession::onConnected() {
	getStream()->setKeepAlive(30);
	return PacketSession::onConnected();
}

void GameServerSession::setGameData(GameData* gameData) {
	GameData* oldGameData = this->gameData;

	if(this->gameData != gameData) {
		this->gameData = gameData;

		if(oldGameData)
			oldGameData->setGameServer(nullptr);
		if(gameData) {
			gameData->setGameServer(this);
			log(LL_Debug, "Set game data to %s\n", gameData->getObjectName());
		}
	}
}

EventChain<PacketSession> GameServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_GA_LOGIN_WITH_LOGOUT::packetID:
			useAutoReconnectFeature = true;
			onServerLogin(static_cast<const TS_GA_LOGIN_WITH_LOGOUT*>(packet), nullptr);
			break;

		case TS_GA_LOGIN_WITH_LOGOUT_EXT::packetID: {
			const TS_GA_LOGIN_WITH_LOGOUT_EXT* loginPacket = static_cast<const TS_GA_LOGIN_WITH_LOGOUT_EXT*>(packet);
			useAutoReconnectFeature = true;
			std::array<uint8_t, 16> guid;
#if !defined(_MSC_VER) || _MSC_VER >= 1900
			static_assert(sizeof(loginPacket->guid) == guid.size(), "GUID size must match");
#endif
			std::copy(std::begin(loginPacket->guid), std::end(loginPacket->guid), std::begin(guid));
			onServerLogin(loginPacket, &guid);
			break;
		}

		case TS_GA_LOGIN::packetID:
			useAutoReconnectFeature = false;
			onServerLogin(static_cast<const TS_GA_LOGIN*>(packet), nullptr);
			break;

		case TS_GA_ACCOUNT_LIST::packetID:
			onAccountList(static_cast<const TS_GA_ACCOUNT_LIST*>(packet));
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

		case TS_GA_SECURITY_NO_CHECK::packetID:
			onSecurityNoCheck(static_cast<const TS_GA_SECURITY_NO_CHECK*>(packet));
			break;

		default:
			log(LL_Debug, "Unknown packet ID: %d, size: %d\n", packet->id, packet->size);
			break;
	}

	return PacketSession::onPacketReceived(packet);
}

void GameServerSession::onServerLogin(const TS_GA_LOGIN* packet, const std::array<uint8_t, 16>* guid) {
	TS_AG_LOGIN_RESULT result;
	TS_MESSAGE::initMessage<TS_AG_LOGIN_RESULT>(&result);

	// to manage non null terminated strings
	std::string localServerName, localServerIp, localScreenshotUrl;

	localServerName = Utils::convertToString(packet->server_name, sizeof(packet->server_name) - 1);
	localServerIp = Utils::convertToString(packet->server_ip, sizeof(packet->server_ip) - 1);
	localScreenshotUrl =
	    Utils::convertToString(packet->server_screenshot_url, sizeof(packet->server_screenshot_url) - 1);

	log(LL_Info,
	    "Server Login: %s[%d] at %s:%d\n",
	    localServerName.c_str(),
	    packet->server_idx,
	    localServerIp.c_str(),
	    packet->server_port);

	if(this->gameData) {
		result.result = TS_RESULT_INVALID_ARGUMENT;
		sendPacket(&result);
		log(LL_Error, "Game server %s already sent login message\n", this->gameData->getServerName().c_str());
		return;
	}

	result.result = TS_RESULT_UNKNOWN;

	GameData* oldGameData = nullptr;
	GameData* gameData = GameData::tryAdd(this,
	                                      packet->server_idx,
	                                      localServerName,
	                                      localServerIp,
	                                      packet->server_port,
	                                      localScreenshotUrl,
	                                      packet->is_adult_server,
	                                      guid,
	                                      &oldGameData);
	if(gameData) {
		result.result = TS_RESULT_SUCCESS;
		log(LL_Debug, "Success\n");
		setGameData(gameData);
		if(!useAutoReconnectFeature)
			gameData->setReady(true);
		else
			gameData->setReady(false);
	} else if(oldGameData) {
		if(useAutoReconnectFeature && guid != nullptr && oldGameData->getServerIdx() == packet->server_idx &&
		   oldGameData->getServerName() == localServerName && oldGameData->getServerIp() == localServerIp &&
		   oldGameData->getServerPort() == packet->server_port &&
		   oldGameData->getServerScreenshotUrl() == localScreenshotUrl &&
		   oldGameData->getIsAdultServer() == packet->is_adult_server && oldGameData->getGuid() == *guid) {
			GameServerSession* gameServer = oldGameData->getGameServer();

			result.result = TS_RESULT_SUCCESS;
			setGameData(oldGameData);
			oldGameData->setReady(false);

			if(gameServer) {
				log(LL_Info,
				    "Received same game info for server %s[%d] but from different connection, dropping the old one "
				    "(maybe halfopen ?)\n",
				    localServerName.c_str(),
				    packet->server_idx);
				gameServer->closeSession();
			} else {
				log(LL_Info, "Game server %s[%d] reconnected\n", localServerName.c_str(), packet->server_idx);
			}
		} else {
			result.result = TS_RESULT_ALREADY_EXIST;
			log(LL_Error, "Failed, server index already used\n");
		}
	}

	sendPacket(&result);
}

void GameServerSession::onAccountList(const TS_GA_ACCOUNT_LIST* packet) {
	if(!gameData) {
		log(LL_Error, "Received account list but game server is not logged on\n");
		return;
	}

	const uint32_t expectedSize = packet->count * sizeof(TS_GA_ACCOUNT_LIST::AccountInfo) + sizeof(TS_GA_ACCOUNT_LIST);
	if(expectedSize != packet->size) {
		log(LL_Error, "Wrong account list packet size, expected %d but got %d\n", expectedSize, packet->size);
		return;
	}

	for(uint8_t i = 0; i < packet->count; i++) {
		TS_GA_ACCOUNT_LIST::AccountInfo accountInfo = packet->accountInfo[i];
		accountInfo.account[sizeof(((TS_GA_ACCOUNT_LIST::AccountInfo*) 0)->account) - 1] = '\0';
		alreadyConnectedAccounts.push_back(accountInfo);
	}
	log(LL_Debug, "Added %d accounts\n", packet->count);

	if(packet->final_packet) {
		// Remove all account connected on this server (they will be recreated after that using the list)
		ClientData::removeServer(gameData);

		for(size_t i = 0; i < alreadyConnectedAccounts.size(); i++) {
			const TS_GA_ACCOUNT_LIST::AccountInfo& accountInfo = alreadyConnectedAccounts[i];
			std::string account = Utils::convertToString(accountInfo.account, sizeof(accountInfo.account) - 1);

			log(LL_Debug, "Adding already connected account %s\n", account.c_str());

			ClientData* clientData = ClientData::tryAddClient(nullptr,
			                                                  account.c_str(),
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
				// Client is connected somewhere else
				TS_AG_KICK_CLIENT msg;

				TS_MESSAGE::initMessage<TS_AG_KICK_CLIENT>(&msg);
				strcpy(msg.account, account.c_str());
				msg.kick_type = TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN;

				log(LL_Info, "Kicked client %s while synchronizing account list\n", account.c_str());
				sendPacket(&msg);
			}
		}
		gameData->setReady(true);
		log(LL_Info, "Synchronized account list with %d accounts\n", (int) alreadyConnectedAccounts.size());
		alreadyConnectedAccounts.clear();
	}
}

void GameServerSession::onServerLogout(const TS_GA_LOGOUT* packet) {
	if(gameData) {
		log(LL_Info, "Server %d Logout\n", gameData->getServerIdx());
		GameData::remove(gameData);
		gameData = nullptr;
	} else {
		log(LL_Error, "Received logout but game server was not logged in\n");
	}
}

void GameServerSession::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	ClientData* client;
	std::string account = Utils::convertToString(packet->account, sizeof(packet->account) - 1);

	client = ClientData::getClient(account);

	TS_ResultCode result = TS_RESULT_ACCESS_DENIED;

	if(!gameData) {
		log(LL_Error, "Received client login for account %s but game server is not logged on\n", account.c_str());
	} else if(account.size() >= sizeof(packet->account)) {
		log(LL_Error,
		    "Account name too long:  \"%s\" (max %d characters)\n",
		    account.c_str(),
		    (int) sizeof(packet->account) - 1);
	} else if(client == nullptr || client->getGameServer() == nullptr) {
		log(LL_Warning, "Client %s login on gameserver but not in client list\n", account.c_str());
	} else if(client->getGameServer() != gameData) {
		log(LL_Warning,
		    "Client %s login on wrong gameserver %s, expected %s\n",
		    account.c_str(),
		    gameData->getServerName().c_str(),
		    client->getGameServer() ? client->getGameServer()->getServerName().c_str() : "none");
	} else if(gameData->isReady() == false) {
		log(LL_Warning, "Client %s login on a not ready gameserver\n", account.c_str());
	} else if(client->oneTimePassword != packet->one_time_key) {
		log(LL_Warning,
		    "Client %s login on gameserver but wrong one time password: expected %" PRIu64 " but received %" PRIu64
		    "\n",
		    account.c_str(),
		    client->oneTimePassword,
		    packet->one_time_key);
	} else if(client->isConnectedToGame()) {
		log(LL_Info, "Client %s login on gameserver but already connected\n", account.c_str());
	} else {
		// To complete
		log(LL_Debug, "Client %s now on gameserver\n", account.c_str());
		result = TS_RESULT_SUCCESS;

		client->connectedToGame();

		LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGIN,
		                         client->accountId,
		                         client->pcBang,
		                         client->eventCode,
		                         gameData->getServerIdx(),
		                         client->age,
		                         0,
		                         0,
		                         0,
		                         0,
		                         0,
		                         0,
		                         client->account.c_str(),
		                         -1,
		                         client->ip,
		                         -1,
		                         0,
		                         0,
		                         0,
		                         0);
	}

	sendClientLoginResult(account.c_str(), result, client);
}

void GameServerSession::onClientLogout(const TS_GA_CLIENT_LOGOUT* packet) {
	std::string account = Utils::convertToString(packet->account, sizeof(packet->account) - 1);

	if(!gameData) {
		log(LL_Error, "Received client logout for account %s but game server is not logged on\n", account.c_str());
		return;
	}

	ClientData* clientData = ClientData::getClient(account);

	log(LL_Debug,
	    "Client %s has been disconnected from gameserver%s\n",
	    account.c_str(),
	    clientData ? "" : " (not a known client)");

	if(!clientData)
		return;

	LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGOUT,
	                         clientData->accountId,
	                         0,
	                         0,
	                         gameData->getServerIdx(),
	                         clientData->pcBang,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         time(nullptr) - clientData->loginTime,
	                         clientData->account.c_str(),
	                         -1,
	                         clientData->ip,
	                         -1,
	                         0,
	                         0,
	                         clientData->kickRequested ? "DKICK" : "NORMAL",
	                         -1);

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
	std::string account = Utils::convertToString(packet->account, sizeof(packet->account) - 1);

	if(!gameData) {
		log(LL_Error, "Received client kick failed for account %s but game server is not logged on\n", account.c_str());
		return;
	}

	ClientData* clientData = ClientData::getClient(account);

	log(LL_Warning, "Client %s kick failed%s\n", account.c_str(), clientData ? " (removing from client list)" : "");

	if(!clientData)
		return;

	LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_LOGOUT,
	                         clientData->accountId,
	                         0,
	                         0,
	                         gameData->getServerIdx(),
	                         clientData->pcBang,
	                         0,
	                         0,
	                         0,
	                         0,
	                         0,
	                         time(nullptr) - clientData->loginTime,
	                         clientData->account.c_str(),
	                         -1,
	                         0,
	                         0,
	                         0,
	                         0,
	                         "FKICK",
	                         -1);

	ClientData::removeClient(clientData);
}

void GameServerSession::onSecurityNoCheck(const TS_GA_SECURITY_NO_CHECK* packet) {
	std::string account = Utils::convertToString(packet->account, sizeof(packet->account) - 1);
	std::string securityNo = Utils::convertToString(packet->security, sizeof(packet->security) - 1);

	int32_t mode = TS_GA_SECURITY_NO_CHECK::SC_NONE;
	if(packet->size >= sizeof(TS_GA_SECURITY_NO_CHECK))
		mode = packet->mode;
	else
		securityNoSendMode = false;

	DB_SecurityNoCheckData::Input input(account, securityNo, mode);
	securityNoCheckQueries.executeDbQuery<DB_SecurityNoCheckData, DB_SecurityNoCheck>(
	    this, &GameServerSession::onSecurityNoCheckResult, input);
}

void GameServerSession::onSecurityNoCheckResult(DB_SecurityNoCheck* query) {
	bool ok = query->getResults().size() == 1;
	DB_SecurityNoCheckData::Input* input = query->getInput();

	if(ok)
		log(LL_Debug, "Security no check for account %s: ok\n", input->account.c_str());
	else
		log(LL_Debug, "Security no check for account %s: wrong\n", input->account.c_str());

	if(securityNoSendMode == true) {
		TS_AG_SECURITY_NO_CHECK securityNoCheckPacket;
		TS_MESSAGE::initMessage(&securityNoCheckPacket);

		strcpy(securityNoCheckPacket.account, input->account.c_str());
		securityNoCheckPacket.mode = input->mode;
		securityNoCheckPacket.result = ok;
		sendPacket(&securityNoCheckPacket);
	} else {
		TS_AG_SECURITY_NO_CHECK_EPIC5 securityNoCheckPacket;
		TS_MESSAGE::initMessage(&securityNoCheckPacket);

		strcpy(securityNoCheckPacket.account, input->account.c_str());
		securityNoCheckPacket.result = ok;
		sendPacket(&securityNoCheckPacket);
	}
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

void GameServerSession::fillClientLoginResult(TS_AG_CLIENT_LOGIN* packet,
                                              const char* account,
                                              TS_ResultCode result,
                                              ClientData* clientData) {
	strncpy(packet->account, account, sizeof(packet->account));
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

void GameServerSession::fillClientLoginExtendedResult(TS_AG_CLIENT_LOGIN_EXTENDED* packet,
                                                      const char* account,
                                                      TS_ResultCode result,
                                                      ClientData* clientData) {
	fillClientLoginResult(packet, account, result, clientData);
	if(result == TS_RESULT_SUCCESS && clientData) {
		memcpy(packet->ip, clientData->ip, INET6_ADDRSTRLEN);
		packet->loginTime = (uint32_t) clientData->loginTime;
	} else {
		packet->ip[0] = '\0';
		packet->loginTime = 0;
	}
}

}  // namespace AuthServer
