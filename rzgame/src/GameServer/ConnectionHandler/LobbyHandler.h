#ifndef LOBBYHANDLER_H
#define LOBBYHANDLER_H

#include "Core/Object.h"
#include "ConnectionHandler.h"
#include "Database/DbQueryJobCallback.h"
#include "../Model/CharacterLight.h"

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
	void onCharacterListResult(DbQueryJob<CharacterLightBinding>* query);

	void onCharacterLogin(const TS_CS_LOGIN* packet);

private:
	DbQueryJobRef characterListQuery;

	std::vector<CharacterLight> characters;
};

} // namespace GameServer

#endif // LOBBYHANDLER_H
