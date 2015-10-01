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
	PlayerLoadingHandler(ClientSession* session, std::unique_ptr<CharacterLight> characterLight);

	void onPacketReceived(const TS_MESSAGE* packet) override;

protected:
	void onCharacterResult(DbQueryJob<CharacterDetailsBinding>* query);

private:
	std::unique_ptr<CharacterLight> characterData;
	std::unique_ptr<CharacterDetails> characterDetails;
	DbQueryJobRef characterQuery;
};

} // namespace GameServer

#endif // GAMEHANDLER_H
