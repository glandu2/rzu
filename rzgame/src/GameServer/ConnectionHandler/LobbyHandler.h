#ifndef LOBBYHANDLER_H
#define LOBBYHANDLER_H

#include "Object.h"
#include "ConnectionHandler.h"
#include "DbQueryJobCallback.h"
#include "../Database/CharacterList.h"

#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_CS_LOGIN.h"

namespace GameServer {

class LobbyHandler : public ConnectionHandler
{
public:
	LobbyHandler(ClientSession* session) : ConnectionHandler(session) {}

	void onPacketReceived(const TS_MESSAGE* packet) override;


protected:
	void onCharacterListQuery(const TS_CS_CHARACTER_LIST* packet);
	void onCharacterListResult(DbQueryJob<Database::CharacterList>* query);

	void onCharacterLogin(const TS_CS_LOGIN* packet);

private:
	DbQueryJobRef characterListQuery;

	struct CharacterId {
		uint32_t sid;
		std::string name;
	};
	std::vector<CharacterId> characters;
};

} // namespace GameServer

#endif // LOBBYHANDLER_H
