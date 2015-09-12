#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include <string.h>
#include "AuthServerSession.h"
#include "../GlobalConfig.h"
#include "Database/DbQueryJobCallback.h"

#include "ConnectionHandler/ConnectionHandler.h"
#include "ConnectionHandler/LobbyHandler.h"

#include "PacketEnums.h"
#include "GameClient/TS_SC_RESULT.h"
#include "AuthGame/TS_GA_CLIENT_LOGIN.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"

#include "GameClient/TS_CS_VERSION.h"

namespace GameServer {

void ClientSession::init() {
}

void ClientSession::deinit() {
}

ClientSession::ClientSession() : authReceived(false), accountId(UINT32_MAX), connectionHandler(nullptr) {
	version = CONFIG_GET()->game.clients.epic.get();
}

ClientSession::~ClientSession() {
	log(LL_Info, "Account %s disconnected\n", account.c_str());

	AuthServerSession::get()->logoutClient(account.c_str(), 0);
	if(connectionHandler)
		delete connectionHandler;
}

void ClientSession::setConnectionHandler(ConnectionHandler* handler) {
	if(connectionHandler)
		delete connectionHandler;
	connectionHandler = handler;
}

void ClientSession::onAccountLoginResult(uint16_t result, std::string account, uint32_t accountId, char nPCBangUser, uint32_t nEventCode, uint32_t nAge, uint32_t nContinuousPlayTime, uint32_t nContinuousLogoutTime) {
	TS_SC_RESULT loginResult;
	TS_MESSAGE::initMessage(&loginResult);

	loginResult.request_msg_id = TS_CS_ACCOUNT_WITH_AUTH::packetID;
	loginResult.result = result;
	loginResult.value = 0;
	sendPacket(&loginResult);

	if(result != TS_RESULT_SUCCESS) {
		log(LL_Warning, "Login failed for account %s: %d\n", account.c_str(), result);
	} else {
		log(LL_Debug, "Login success for account %s\n", account.c_str());
		this->accountId = accountId;
		this->account = account;
		setConnectionHandler(new LobbyHandler(this));
	}
}

void ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	if(accountId == UINT32_MAX) {
		switch(packet->id) {
			case TS_CS_ACCOUNT_WITH_AUTH::packetID:
				packet->process(this, &ClientSession::onAccountWithAuth, version);
				break;
		}
	} else if(connectionHandler) {
		connectionHandler->onPacketReceived(packet);
	} else {
		log(LL_Warning, "Account %s authenticated but no connection handler !\n", account.c_str());
	}
}

void ClientSession::onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH* packet) {
	if(authReceived == false) {
		std::string account = Utils::convertToString(packet->account, sizeof(packet->account) - 1);

		authReceived = true;
		AuthServerSession::get()->loginClient(this, account, packet->one_time_key);
	} else {
		log(LL_Warning, "Client already sent a auth packet, closing connection\n");
		abortSession();
	}
}

} //namespace UploadServer
