#include "GameSession.h"
#include "AuthSession.h"
#include "Core/EventLoop.h"
#include <algorithm>
#include "AuctionWorker.h"
#include <time.h>
#include "Config/ConfigParamVal.h"

#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_CS_LOGOUT.h"
#include "GameClient/TS_TIMESYNC.h"
#include "GameClient/TS_SC_DISCONNECT_DESC.h"
#include "GameClient/TS_CS_UPDATE.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_SC_GAME_TIME.h"
#include "TS_CS_AUCTION_SEARCH.h"
#include "TS_SC_AUCTION_SEARCH.h"
#include "GameClient/TS_SC_RESULT.h"

GameSession::GameSession(AuctionWorker *auctionWorker, const std::string& playername, cval<int>& ggRecoTime)
	: auctionWorker(auctionWorker),
	  playername(playername),
	  connectedInGame(false),
	  handle(0),
	  rappelzTimeOffset(0),
	  epochTimeOffset(0),
	  ggRecoTime(ggRecoTime)
{
}

void GameSession::onGameConnected() {
	int ggRecoTimeValue = ggRecoTime.get();
	if(ggRecoTimeValue > 10)
		ggPreventionRecoTimer.start(this, &GameSession::onGGPreventionTimerExpired, (ggRecoTimeValue - 10)*1000, 0);
	if(ggRecoTimeValue > 0) {
		log(LL_Debug, "Starting GG timer: %ds\n", ggRecoTimeValue);
		ggRecoTimer.start(this, &GameSession::onGGTimerExpired, ggRecoTimeValue*1000, 0);
	}
	shouldReconnect = false;

	epochTimeOffset = rappelzTimeOffset = 0;

	TS_CS_CHARACTER_LIST charlistPkt;
	TS_MESSAGE::initMessage<TS_CS_CHARACTER_LIST>(&charlistPkt);
	strcpy(charlistPkt.account, auth->getAccountName().c_str());
	sendPacket(&charlistPkt);
}

void GameSession::close() {
	TS_CS_LOGOUT logoutPkt;
	sendPacket(logoutPkt, EPIC_LATEST);
	closeSession();
}

void GameSession::onGameDisconnected() {
	updateTimer.stop();
	ggPreventionRecoTimer.stop();
	ggRecoTimer.stop();
	setConnected(false);
}

void GameSession::onGGPreventionTimerExpired() {
	log(LL_Info, "GG timeout, scheduling reconnect\n");
	shouldReconnect = true;
}

void GameSession::onGGTimerExpired() {
	log(LL_Info, "GG timeout, reconnecting\n");
	close();
}

void GameSession::onUpdatePacketExpired() {
	if(!connectedInGame)
		return;

	TS_CS_UPDATE updatPkt;

	updatPkt.handle = handle;
	updatPkt.time = getRappelzTime() + rappelzTimeOffset;
	updatPkt.epoch_time = uint32_t(time(NULL) + epochTimeOffset);

	sendPacket(updatPkt, EPIC_LATEST);
}

uint32_t GameSession::getRappelzTime()
{
	return uint32_t(uv_hrtime() / (10 * 1000 * 1000));
}

void GameSession::onGamePacketReceived(const TS_MESSAGE *packet) {
	switch(packet->id) {
		case TS_SC_CHARACTER_LIST::packetID:
			packet->process(this, &GameSession::onCharacterList, EPIC_LATEST);
			break;

		case_packet_is(TS_SC_LOGIN_RESULT)
			packet->process(this, &GameSession::onCharacterLoginResult, EPIC_LATEST);
			break;

		case TS_SC_AUCTION_SEARCH::packetID:
			auctionWorker->onAuctionSearchResult((const TS_SC_AUCTION_SEARCH*)packet);
			if(shouldReconnect)
				abortSession();
			break;

		case TS_SC_RESULT::packetID: {
			const TS_SC_RESULT* resultPacket = (const TS_SC_RESULT*)packet;
			if(resultPacket->request_msg_id == TS_CS_AUCTION_SEARCH::packetID) {
				auctionWorker->onAuctionSearchFailed(resultPacket->result);
			}
			break;
		}

		case TS_SC_DISCONNECT_DESC::packetID:
			setConnected(false);
			abortSession();
			break;

		case TS_TIMESYNC::packetID: {
			const TS_TIMESYNC* serverTime = (const TS_TIMESYNC*)packet;
			rappelzTimeOffset = serverTime->time - getRappelzTime();

			TS_TIMESYNC timeSyncPkt;

			TS_MESSAGE::initMessage<TS_TIMESYNC>(&timeSyncPkt);
			timeSyncPkt.time = serverTime->time;
			sendPacket(&timeSyncPkt);

			if(epochTimeOffset == 0) {
				TS_CS_GAME_TIME gameTimePkt;
				sendPacket(gameTimePkt, EPIC_LATEST);
			}
			break;
		}

		case TS_SC_GAME_TIME::packetID: {
			const TS_SC_GAME_TIME* serverTime = (const TS_SC_GAME_TIME*)packet;
			if(epochTimeOffset == 0)
				updateTimer.start(this, &GameSession::onUpdatePacketExpired, 5000, 5000);

			rappelzTimeOffset = serverTime->t - getRappelzTime();
			epochTimeOffset = uint32_t(serverTime->game_time - time(NULL));
			break;
		}
	}
}

void GameSession::onCharacterList(const TS_SC_CHARACTER_LIST* packet) {
	TS_CS_LOGIN loginPkt;
	TS_TIMESYNC timeSyncPkt;
	bool characterInList = false;

	TS_MESSAGE::initMessage<TS_TIMESYNC>(&timeSyncPkt);

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

	loginPkt.name = playername;
	loginPkt.race = 0;
	sendPacket(loginPkt, EPIC_LATEST);

	timeSyncPkt.time = 0;
	sendPacket(&timeSyncPkt);
}

void GameSession::onCharacterLoginResult(const TS_SC_LOGIN_RESULT *packet) {
	handle = packet->handle;
	log(LL_Info, "Connected with character %s\n", playername.c_str());
	setConnected(true);
}

void GameSession::setConnected(bool connected)
{
	if(connected && !connectedInGame)
		auctionWorker->onConnected();
	else if(!connected && connectedInGame)
		auctionWorker->onDisconnected();
	connectedInGame = connected;
}

void GameSession::auctionSearch(int category_id, int page)
{
	TS_CS_AUCTION_SEARCH auctionSearchPacket;
	TS_MESSAGE::initMessage<TS_CS_AUCTION_SEARCH>(&auctionSearchPacket);

	auctionSearchPacket.category_id = category_id;
	auctionSearchPacket.sub_category_id = -1;
	memset(auctionSearchPacket.keyword, 0, sizeof(auctionSearchPacket.keyword));
	auctionSearchPacket.page_num = page;
	auctionSearchPacket.is_equipable = false;

	sendPacket(&auctionSearchPacket);
}
