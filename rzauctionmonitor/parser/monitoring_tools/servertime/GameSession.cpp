#include "GameSession.h"
#include "Cipher/RzHashReversible256.h"
#include "Core/Utils.h"
#include "NetSession/ClientAuthSession.h"
#include <algorithm>
#include <time.h>

#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_CS_UPDATE.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_CHAT_LOCAL.h"
#include "GameClient/TS_SC_DISCONNECT_DESC.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_GAME_TIME.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_TIMESYNC.h"
#include "Packet/PacketEpics.h"

GameSession::GameSession(const std::string& ip,
                         uint16_t port,
                         const std::string& account,
                         const std::string& password,
                         int serverIdx,
                         const std::string& playername,
                         int epic,
                         int delayTime,
                         int ggRecoTime)
    : AutoClientSession(ip, port, account, password, serverIdx, playername, epic, true, delayTime, ggRecoTime) {}

void GameSession::onClockExpired() {
	uint32_t gameTime = getGameTime() * 10;
	int days, hour, minute, second, milisecond;

	days = gameTime / (86400 * 1000);
	gameTime %= 86400 * 1000;

	hour = gameTime / (3600 * 1000);
	minute = (gameTime % (3600 * 1000)) / 60000;
	second = (gameTime % 60000) / 1000;
	milisecond = gameTime % 1000;

	log(LL_Info, "Game time: %d days %02d:%02d:%02d.%03d\n", days, hour, minute, second, milisecond);
	waitNextGameSecond();
}

void GameSession::onCheckAuctionExpired() {
	//	auctionQueryWork.queue(this, d);
}

void GameSession::waitNextGameSecond() {
	uint32_t positionInSecond = (getGameTime() * 10) % 1000;
	clockTimer.start(this, &GameSession::onClockExpired, 1000 - positionInSecond, 0);
}

void GameSession::onPacketReceived(const TS_MESSAGE* packet, EventTag<AutoClientSession>) {
	switch(packet->id) {
		case_packet_is(TS_SC_ENTER) packet->process(this, &GameSession::onEnter, packetVersion);
		break;

		case TS_TIMESYNC::packetID:
			packet->process(this, &GameSession::onTimeSync, packetVersion);
			break;
	}
}

void GameSession::onEnter(const TS_SC_ENTER* packet) {
	if(packet->objType == EOT_Player) {
		playerNames[packet->handle] = packet->playerInfo.name;
	}
}

void GameSession::onTimeSync(const TS_TIMESYNC* packet) {
	log(LL_Info, "Time synchronization\n");
	waitNextGameSecond();
}
