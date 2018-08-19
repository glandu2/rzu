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

GameSession::GameSession(const std::string& playername, Log* packetLog, int epic)
    : ClientGameSession(epic),
      playername(playername),
      connectedInGame(false),
      handle(0),
      rappelzTimeOffset(0),
      epochTimeOffset(0) {}

void GameSession::onGameConnected() {
	epochTimeOffset = rappelzTimeOffset = 0;

	TS_CS_CHARACTER_LIST charlistPkt;
	charlistPkt.account = auth->getAccountName();
	sendPacket(charlistPkt);
}

void GameSession::onGameDisconnected() {
	updateTimer.stop();
	connectedInGame = false;
}

void GameSession::onUpdatePacketExpired() {
	if(!connectedInGame)
		return;

	TS_CS_UPDATE updatPkt;

	updatPkt.handle = handle;
	updatPkt.time = getRappelzTime() + rappelzTimeOffset;
	updatPkt.epoch_time = uint32_t(time(NULL) + epochTimeOffset);

	sendPacket(updatPkt);
}

void GameSession::onClockExpired() {
	uint64_t rappelzTime = (getRappelzTime() + rappelzTimeOffset) * 10;
	int days, hour, minute, second, milisecond;

	days = rappelzTime / (86400 * 1000);
	rappelzTime %= 86400 * 1000;

	hour = rappelzTime / (3600 * 1000);
	minute = (rappelzTime % (3600 * 1000)) / 60000;
	second = (rappelzTime % 60000) / 1000;
	milisecond = rappelzTime % 1000;

	log(LL_Info, "Rappelz time: %d days %02d:%02d:%02d.%03d\n", days, hour, minute, second, milisecond);
	waitNextRappelzSecond();
}

void GameSession::onCheckAuctionExpired() {
	//	auctionQueryWork.queue(this, d);
}

uint32_t GameSession::getRappelzTime() {
	return uint32_t(uv_hrtime() / (10 * 1000 * 1000));
}

void GameSession::waitNextRappelzSecond() {
	uint32_t positionInSecond = ((getRappelzTime() + rappelzTimeOffset) * 10) % 1000;
	clockTimer.start(this, &GameSession::onClockExpired, 1000 - positionInSecond, 0);
}

void GameSession::onGamePacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_SC_CHARACTER_LIST::packetID:
			packet->process(this, &GameSession::onCharacterList, version);
			break;

			case_packet_is(TS_SC_LOGIN_RESULT) packet->process(this, &GameSession::onCharacterLoginResult, version);
			break;

			case_packet_is(TS_SC_ENTER) packet->process(this, &GameSession::onEnter, version);
			break;

		case TS_SC_DISCONNECT_DESC::packetID:
			connectedInGame = false;
			abortSession();
			break;

		case TS_TIMESYNC::packetID:
			packet->process(this, &GameSession::onTimeSync, version);
			break;

		case TS_SC_GAME_TIME::packetID:
			packet->process(this, &GameSession::onGameTime, version);
			break;
	}
}

void GameSession::onCharacterList(const TS_SC_CHARACTER_LIST* packet) {
	bool characterInList = false;

	log(LL_Debug, "Character list: \n");
	for(size_t i = 0; i < packet->characters.size(); i++) {
		log(LL_Debug, " - %s\n", packet->characters[i].name.c_str());
		if(playername == packet->characters[i].name)
			characterInList = true;
	}

	if(!characterInList) {
		log(LL_Warning, "Character \"%s\" not in character list: \n", playername.c_str());
		for(size_t i = 0; i < packet->characters.size(); i++) {
			log(LL_Warning, " - %s\n", packet->characters[i].name.c_str());
		}
	}

	TS_CS_LOGIN loginPkt;
	loginPkt.name = playername;
	loginPkt.race = 0;
	RzHashReversible256::generatePayload(loginPkt);
	sendPacket(loginPkt);

	TS_TIMESYNC timeSyncPkt;
	timeSyncPkt.time = 0;
	sendPacket(timeSyncPkt);

	updateTimer.start(this, &GameSession::onUpdatePacketExpired, 5000, 5000);
}

void GameSession::onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet) {
	handle = packet->handle;
	connectedInGame = true;
	log(LL_Info, "Connected with character %s\n", playername.c_str());
}

void GameSession::onEnter(const TS_SC_ENTER* packet) {
	if(packet->objType == EOT_Player) {
		playerNames[packet->handle] = packet->playerInfo.szName;
	}
}

void GameSession::onTimeSync(const TS_TIMESYNC* packet) {
	rappelzTimeOffset = packet->time - getRappelzTime();
	log(LL_Info, "Time synchronization\n");
	waitNextRappelzSecond();

	TS_TIMESYNC timeSyncPkt;
	timeSyncPkt.time = packet->time;
	sendPacket(timeSyncPkt);

	if(epochTimeOffset == 0) {
		TS_CS_GAME_TIME gameTimePkt;
		sendPacket(gameTimePkt);
	}
}

void GameSession::onGameTime(const TS_SC_GAME_TIME* packet) {
	log(LL_Info, "Time update\n");
	rappelzTimeOffset = packet->t - getRappelzTime();
	epochTimeOffset = int32_t(packet->game_time - time(NULL));
}
