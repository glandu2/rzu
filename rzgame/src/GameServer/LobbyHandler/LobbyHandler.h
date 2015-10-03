#ifndef LOBBYHANDLER_H
#define LOBBYHANDLER_H

#include "Core/Object.h"
#include "../ConnectionHandler.h"
#include "Database/DbQueryJobCallback.h"
#include "../Model/CharacterLight.h"
#include "CheckCharacterName.h"

#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_CS_CHECK_CHARACTER_NAME.h"
#include "GameClient/TS_CS_CREATE_CHARACTER.h"
#include "GameClient/TS_CS_DELETE_CHARACTER.h"

namespace GameServer {

class LobbyHandler : public ConnectionHandler
{
	DECLARE_CLASS(GameServer::LobbyHandler)
public:
	LobbyHandler(ClientSession* session) : ConnectionHandler(session) {}

	void onPacketReceived(const TS_MESSAGE* packet) override;


protected:
	void onCharacterListQuery(const TS_CS_CHARACTER_LIST* packet);
	void onCharacterListResult(DbQueryJob<CharacterLightBinding>* query);

	void onCheckCharacterName(const TS_CS_CHECK_CHARACTER_NAME* packet);
	void onCheckCharacterNameExistsResult(DbQueryJob<CheckCharacterNameBinding>* query);

	void onCreateCharacter(const TS_CS_CREATE_CHARACTER* packet);
	void onDeleteCharacter(const TS_CS_DELETE_CHARACTER* packet);

	void onCharacterLogin(const TS_CS_LOGIN* packet);

protected:
	static bool checkTextAgainstEncoding(const std::string &text);

private:
	DbQueryJobRef characterListQuery;

	std::vector<std::unique_ptr<CharacterLight>> characters;
};

} // namespace GameServer

#endif // LOBBYHANDLER_H
