#include "TimeManager.h"
#include "ClientSession.h"
#include "Core/Utils.h"
#include "GameClient/TS_SC_GAME_TIME.h"

namespace GameServer {

rztime_t TimeManager::getRzTime() {
	static uint64_t baseTime = Utils::getTimeInMsec();
	return static_cast<rztime_t>((Utils::getTimeInMsec() - baseTime) / 10);
}

void TimeManager::sendGameTime(ClientSession* session) {
	TS_SC_GAME_TIME gameTime;
	gameTime.t = getRzTime();
	gameTime.game_time = time(nullptr);
	session->sendPacket(gameTime);
}

}  // namespace GameServer
