#ifndef GAMESERVER_CLIENTSESSION_H
#define GAMESERVER_CLIENTSESSION_H

#include "PacketSession.h"
#include "EncryptedSession.h"
#include <unordered_map>
#include "Database/CharacterList.h"
#include "DbQueryJobCallback.h"

#include "Packets/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "Packets/TS_CS_CHARACTER_LIST.h"

namespace GameServer {

class ClientSession : public EncryptedSession<PacketSession>
{
	DECLARE_CLASS(GameServer::ClientSession)
public:
	static void init();
	static void deinit();

	ClientSession();

	void onAccountLoginResult(uint16_t result, std::string account, uint32_t accountId, char nPCBangUser, uint32_t nEventCode, uint32_t nAge, uint32_t nContinuousPlayTime, uint32_t nContinuousLogoutTime);

	void onCharacterList(DbQueryJob<Database::CharacterList>* query);

protected:
	~ClientSession();

	void onPacketReceived(const TS_MESSAGE* packet);

	void onAccountWithAuth(TS_CS_ACCOUNT_WITH_AUTH& packet);
	void onCharacterListQuery(const TS_CS_CHARACTER_LIST* packet);

private:
	std::string account;
	uint32_t accountId;
	DbQueryJobRef characterListQuery;
};

}

#endif // GAMESERVER_CLIENTSESSION_H
