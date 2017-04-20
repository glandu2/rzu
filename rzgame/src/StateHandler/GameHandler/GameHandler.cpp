#include "GameHandler.h"
#include "ClientSession.h"

#include "GameClient/TS_TIMESYNC.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_SC_MOVE.h"

#include "Component/Inventory/Inventory.h"
#include "Component/Time/TimeManager.h"

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
			session->sendPacket(response);
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

		case_packet_is(TS_CS_MOVE_REQUEST)
			packet->process(this, &GameHandler::onMoveRequest, session->getVersion());
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

void GameHandler::onMoveRequest(const TS_CS_MOVE_REQUEST *packet) {
	TS_SC_MOVE movePkt;
	movePkt.start_time = TimeManager::getRzTime();
	movePkt.handle = character->handle;
	movePkt.tlayer = 0;
	movePkt.speed = -128;

	movePkt.move_infos.reserve(packet->move_infos.size());
	for(size_t i = 0; i < packet->move_infos.size(); i++) {
		MOVE_INFO moveInfo;
		const MOVE_REQUEST_INFO& moveRequestInfo = packet->move_infos[i];
		moveInfo.tx = moveRequestInfo.tx;
		moveInfo.ty = moveRequestInfo.ty;
		movePkt.move_infos.push_back(moveInfo);
	}

	session->sendPacket(movePkt);
}

} // namespace GameServer
