#pragma once

#include "Component/Character/Character.h"
#include "ConnectionHandler.h"
#include "GameClient/TS_CS_MOVE_REQUEST.h"
#include "GameClient/TS_CS_PUTOFF_ITEM.h"
#include "GameClient/TS_CS_PUTON_ITEM.h"
#include <memory>

namespace GameServer {

class ClientSession;

class GameHandler : public ConnectionHandler {
	DECLARE_CLASS(GameServer::GameHandler)
public:
	GameHandler(ClientSession* session, std::unique_ptr<Character> character);

	void onPacketReceived(const TS_MESSAGE* packet) override;

protected:
	void onPutonItem(const TS_CS_PUTON_ITEM* packet);
	void onPutoffItem(const TS_CS_PUTOFF_ITEM* packet);
	void onMoveRequest(const TS_CS_MOVE_REQUEST* packet);

private:
	std::unique_ptr<Character> character;
};

}  // namespace GameServer

