#ifndef PLAYERLOADINGHANDLER_H
#define PLAYERLOADINGHANDLER_H

#include "ConnectionHandler.h"
#include "Database/DB_Character.h"
#include "Database/DB_Item.h"
#include "Database/DbQueryJobRef.h"
#include <memory>

namespace GameServer {

class Character;

class PlayerLoadingHandler : public ConnectionHandler
{
	DECLARE_CLASS(GameServer::PlayerLoadingHandler)
public:
	PlayerLoadingHandler(ClientSession* session, game_sid_t sid);

	void onPacketReceived(const TS_MESSAGE* packet) override;

protected:
	void onCharacterResult(DbQueryJob<DB_CharacterBinding>* query);
	void onItemListResult(DbQueryJob<DB_ItemBinding>* query);

private:
	DbQueryJobRef characterQuery;
	std::unique_ptr<Character> character;
};

} // namespace GameServer

#endif // PLAYERLOADINGHANDLER_H
