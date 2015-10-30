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

		case TS_CS_PUTON_ITEM::packetID:
			packet->process(this, &GameHandler::onPutonItem, session->getVersion());
			break;

		case TS_CS_PUTOFF_ITEM::packetID:
			packet->process(this, &GameHandler::onPutoffItem, session->getVersion());
			break;
	}
}

void GameHandler::onPutonItem(const TS_CS_PUTON_ITEM *packet) {
	character->inventory.equipItem(packet->item_handle, (Inventory::ItemWearType)packet->position);
	character->sendEquip();
}

void GameHandler::onPutoffItem(const TS_CS_PUTOFF_ITEM *packet) {
	character->inventory.unequipItem((Inventory::ItemWearType)packet->position);
	character->sendEquip();
}

} // namespace GameServer
