#ifndef GAMEHANDLER_H
#define GAMEHANDLER_H

#include "ConnectionHandler.h"
#include "../Model/Character.h"
#include "Database/DbQueryJobCallback.h"

namespace GameServer {

class GameHandler : public ConnectionHandler
{
public:
	GameHandler(ClientSession* session, const CharacterLight& characterLight);

	void onPacketReceived(const TS_MESSAGE* packet) override;

protected:
	void onCharacterResult(DbQueryJob<CharacterDetailsBinding>* query);

private:
	CharacterLight characterData;
	CharacterDetails characterDetails;
	DbQueryJobRef characterQuery;
};

} // namespace GameServer

#endif // GAMEHANDLER_H
