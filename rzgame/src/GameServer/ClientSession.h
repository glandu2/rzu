#ifndef GAMESERVER_CLIENTSESSION_H
#define GAMESERVER_CLIENTSESSION_H

#include "NetSession/PacketSession.h"
#include "NetSession/EncryptedSession.h"
#include <unordered_map>
#include <memory>
#include "ConnectionHandler/ConnectionHandler.h"

#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_SC_RESULT.h"

class CharacterLight;

namespace GameServer {

class ClientSession : public EncryptedSession<PacketSession>
{
	DECLARE_CLASS(GameServer::ClientSession)
public:
	static void init();
	static void deinit();

	ClientSession();

	uint32_t getAccountId() { return accountId; }
	std::string getAccount() { return account; }
	int getVersion() { return version; }

	void onAccountLoginResult(uint16_t result, std::string account, uint32_t accountId, char nPCBangUser, uint32_t nEventCode, uint32_t nAge, uint32_t nContinuousPlayTime, uint32_t nContinuousLogoutTime);
	void lobbyExitResult(std::unique_ptr<CharacterLight> characterData);
	void playerLoadingResult(TS_ResultCode result);

	template<class T>
	void sendResult(uint16_t result, int32_t value) {
		TS_SC_RESULT resultPacket;
		TS_MESSAGE::initMessage(&resultPacket);
		resultPacket.request_msg_id = T::packetId;
		resultPacket.result = result;
		resultPacket.value = value;
		sendPacket(&resultPacket);
	}

protected:
	~ClientSession();

	void onPacketReceived(const TS_MESSAGE* packet);

	void onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH *packet);

	void setConnectionHandler(ConnectionHandler* newConnectionHandler);

private:
	int version;
	bool authReceived;
	std::string account;
	uint32_t accountId;
	std::unique_ptr<ConnectionHandler> connectionHandler;
	std::unique_ptr<ConnectionHandler> oldConnectionHandler;
};

}

#endif // GAMESERVER_CLIENTSESSION_H
