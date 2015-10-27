#include "GameHandler.h"
#include "../ClientSession.h"
#include "../TimeManager.h"

#include "GameClient/TS_TIMESYNC.h"
#include "GameClient/TS_CS_GAME_TIME.h"

namespace GameServer {

GameHandler::GameHandler(ClientSession *session, std::unique_ptr<Character> character)
	: ConnectionHandler(session), character(std::move(character))
{
}

void GameHandler::onPacketReceived(const TS_MESSAGE *packet) {
	switch(packet->id) {
		case TS_TIMESYNC::packetID: {
			TS_TIMESYNC response;
			response.time = TimeManager::getRzTime();
			session->sendPacket(&response);
			break;
		}

		case TS_CS_GAME_TIME::packetID:
			TimeManager::sendGameTime(session);
			break;
	}
}

} // namespace GameServer
