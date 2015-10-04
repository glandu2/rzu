#ifndef LOBBYHANDLER_H
#define LOBBYHANDLER_H

#include "Core/Object.h"
#include "../ConnectionHandler.h"
#include "Database/DbQueryJobCallback.h"
#include "../Model/CharacterLight.h"
#include "../Model/CharacterWearInfo.h"

#include "CheckCharacterName.h"
#include "CreateCharacter.h"
#include "DeleteCharacter.h"

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
	LobbyHandler(ClientSession* session);

	void onPacketReceived(const TS_MESSAGE* packet) override;


protected:
	void onCharacterListQuery(const TS_CS_CHARACTER_LIST* packet);
	void onCharacterListResult(DbQueryJob<CharacterLightBinding>* query);
	void onCharacterWearInfoResult(DbQueryJob<CharacterWearInfoBinding>* query);

	void onCheckCharacterName(const TS_CS_CHECK_CHARACTER_NAME* packet);
	void onCheckCharacterNameExistsResult(DbQueryJob<CheckCharacterNameBinding>* query);

	void onCreateCharacter(const TS_CS_CREATE_CHARACTER* packet);
	void onCreateCharacterResult(DbQueryJob<CreateCharacterBinding>* query);

	void onDeleteCharacter(const TS_CS_DELETE_CHARACTER* packet);
	void onDeleteCharacterResult(DbQueryJob<DeleteCharacterBinding>* query);

	void onCharacterLogin(const TS_CS_LOGIN* packet);

protected:
	static bool checkTextAgainstEncoding(const std::string &text);

private:
	DbQueryJobRef lobbyQueries;

	bool charactersPopulated;
	std::vector<std::unique_ptr<CharacterLight>> characters;
	std::string lastValidatedCharacterName;
};

} // namespace GameServer

#endif // LOBBYHANDLER_H
