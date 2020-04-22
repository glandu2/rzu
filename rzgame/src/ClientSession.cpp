#include "ClientSession.h"
#include "AuthServerSession.h"
#include "Config/GlobalConfig.h"
#include "Database/DbQueryJobCallback.h"
#include <string.h>

#include "Component/Character/Character.h"
#include "ConnectionHandler.h"
#include "StateHandler/GameHandler/GameHandler.h"
#include "StateHandler/LobbyHandler/LobbyHandler.h"
#include "StateHandler/PlayerLoadingHandler/PlayerLoadingHandler.h"

#include "AuthGame/TS_GA_CLIENT_LOGIN.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_PROPERTY.h"
#include "GameClient/TS_SC_RESULT.h"
#include "PacketEnums.h"

#include "GameClient/TS_CS_VERSION.h"

namespace GameServer {

void ClientSession::init() {}

void ClientSession::deinit() {}

ClientSession::ClientSession()
    : EncryptedSession<PacketSession>(
          SessionType::GameClient, SessionPacketOrigin::Server, CONFIG_GET()->game.clients.epic.get()),
      authReceived(false),
      accountId(UINT32_MAX) {}

ClientSession::~ClientSession() {
	log(LL_Info, "Account %s disconnected\n", account.c_str());

	AuthServerSession::get()->logoutClient(account.c_str(), 0);
}

void ClientSession::setConnectionHandler(ConnectionHandler* newConnectionHandler) {
	oldConnectionHandler = std::move(connectionHandler);
	connectionHandler.reset(newConnectionHandler);
}

void ClientSession::onAccountLoginResult(uint16_t result,
                                         std::string account,
                                         uint32_t accountId,
                                         char nPCBangUser,
                                         uint32_t nEventCode,
                                         uint32_t nAge,
                                         uint32_t nContinuousPlayTime,
                                         uint32_t nContinuousLogoutTime) {
	TS_SC_RESULT loginResult;

	loginResult.request_msg_id = TS_CS_ACCOUNT_WITH_AUTH::packetID;
	loginResult.result = result;
	loginResult.value = 0;
	sendPacket(loginResult);

	if(result != TS_RESULT_SUCCESS) {
		log(LL_Warning, "Login failed for account %s: %d\n", account.c_str(), result);
	} else {
		log(LL_Debug, "Login success for account %s\n", account.c_str());
		this->accountId = accountId;
		this->account = account;
		setConnectionHandler(new LobbyHandler(this));
	}
}

EventChain<PacketSession> ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	if(accountId == UINT32_MAX) {
		switch(packet->id) {
			case TS_CS_ACCOUNT_WITH_AUTH::packetID:
				packet->process(this, &ClientSession::onAccountWithAuth, packetVersion);
				break;
		}
	} else if(connectionHandler) {
		connectionHandler->onPacketReceived(packet);
		oldConnectionHandler.reset();  // free old connection handler if it has changed
	} else {
		log(LL_Warning, "Account %s authenticated but no connection handler !\n", account.c_str());
	}

	return PacketSession::onPacketReceived(packet);
}

void ClientSession::onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH* packet) {
	if(authReceived == false) {
		authReceived = true;
		AuthServerSession::get()->loginClient(this, packet->account, packet->one_time_key);
	} else {
		log(LL_Warning, "Client already sent a auth packet, closing connection\n");
		abortSession();
	}
}

void ClientSession::lobbyExitResult(TS_ResultCode result, std::unique_ptr<CharacterLight> characterData) {
	if(!characterData || result != TS_RESULT_SUCCESS) {
		TS_SC_LOGIN_RESULT loginResult = {};
		loginResult.result = result;
		sendPacket(loginResult);
		abortSession();
	} else {
		setConnectionHandler(new PlayerLoadingHandler(this, characterData->sid));
	}
}

void ClientSession::playerLoadingResult(TS_ResultCode result, std::unique_ptr<Character> character) {
	if(result != TS_RESULT_SUCCESS) {
		abortSession();
	} else {
		setConnectionHandler(new GameHandler(this, std::move(character)));
	}
}

void ClientSession::sendResult(uint16_t id, uint16_t result, int32_t value) {
	TS_SC_RESULT resultPacket;
	resultPacket.request_msg_id = id;
	resultPacket.result = result;
	resultPacket.value = value;
	sendPacket(resultPacket);
}

void ClientSession::sendResult(const TS_MESSAGE* originalPacket, uint16_t result, int32_t value) {
	sendResult(originalPacket->id, result, value);
}

void ClientSession::sendProperty(game_handle_t handle, const char* name, int64_t value) {
	TS_SC_PROPERTY property;
	property.handle = handle;
	property.name = name;
	property.is_number = true;
	property.value = value;
	sendPacket(property);
}

void ClientSession::sendProperty(game_handle_t handle, const char* name, const std::string& value) {
	TS_SC_PROPERTY property;
	property.handle = handle;
	property.name = name;
	property.is_number = false;
	property.string_value = value;
	sendPacket(property);
}

}  // namespace GameServer
