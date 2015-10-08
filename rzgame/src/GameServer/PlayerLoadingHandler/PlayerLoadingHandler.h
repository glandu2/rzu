#ifndef GAMEHANDLER_H
#define GAMEHANDLER_H

#include "../ConnectionHandler.h"
#include "../Model/Character.h"
#include "Database/DbQueryJobCallback.h"
#include <memory>

namespace GameServer {

class PlayerLoadingHandler : public ConnectionHandler
{
	DECLARE_CLASS(GameServer::PlayerLoadingHandler)
public:
	PlayerLoadingHandler(ClientSession* session, uint64_t sid);

	void onPacketReceived(const TS_MESSAGE* packet) override;

protected:
	void onCharacterResult(DbQueryJob<CharacterBinding>* query);

private:
	std::unique_ptr<Character> characterData;
	DbQueryJobRef characterQuery;
};

} // namespace GameServer

#endif // GAMEHANDLER_H
