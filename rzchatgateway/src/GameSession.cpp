#include "GameSession.h"
#include "ChatAuthSession.h"
#include "IrcClient.h"
#include <algorithm>
#include <time.h>

#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_CHAT_LOCAL.h"
#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_TIMESYNC.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_DISCONNECT_DESC.h"
#include "GameClient/TS_CS_UPDATE.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_SC_GAME_TIME.h"

GameSession::GameSession(const std::string& playername, bool enableGateway, Log *packetLog)
	: playername(playername),
	  ircClient(nullptr),
	  enableGateway(enableGateway),
	  connectedInGame(false),
	  handle(0),
	  rappelzTimeOffset(0),
	  epochTimeOffset(0)
{
}

void GameSession::onGameConnected() {
	updateTimer.start(this, &GameSession::onUpdatePacketExpired, 5000, 5000);
	epochTimeOffset = rappelzTimeOffset = 0;

	TS_CS_CHARACTER_LIST charlistPkt;
	TS_MESSAGE::initMessage<TS_CS_CHARACTER_LIST>(&charlistPkt);
	strcpy(charlistPkt.account, auth->getAccountName().c_str());
	sendPacket(&charlistPkt);
}

void GameSession::setGameServerName(std::string name) {
	if(ircClient->isConnected() == false)
		ircClient->connect(name);
}

void GameSession::onGameDisconnected() {
	updateTimer.stop();
	connectedInGame = false;
	ircClient->sendMessage("", "\001ACTION disconnected from the game server\001");
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

		case_packet_is(TS_SC_ENTER)
			packet->process(this, &GameSession::onEnter, EPIC_LATEST);
			break;

		case TS_SC_CHAT_LOCAL::packetID: {
			TS_SC_CHAT_LOCAL* chatPkt = (TS_SC_CHAT_LOCAL*) packet;
			std::string msg = std::string(chatPkt->message, chatPkt->len);
			std::unordered_map<unsigned int, std::string>::iterator it = playerNames.find(chatPkt->handle);
			std::string playerName = "Unknown";

			if(chatPkt->handle == handle)
				break;

			if(it != playerNames.end())
				playerName = it->second;

			ircClient->sendMsgToIRC(chatPkt->type, playerName.c_str(), msg);
			break;
		}
		case TS_SC_CHAT::packetID: {
			TS_SC_CHAT* chatPkt = (TS_SC_CHAT*) packet;
			std::string msg = std::string(chatPkt->message, chatPkt->len);
			ircClient->sendMsgToIRC(chatPkt->type, chatPkt->szSender, msg);
			break;
		}

		case TS_SC_DISCONNECT_DESC::packetID:
			connectedInGame = false;
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
			rappelzTimeOffset = serverTime->t - getRappelzTime();
			epochTimeOffset = int32_t(serverTime->game_time - time(NULL));
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

	ircClient->sendMessage("", "\001ACTION is connected to the game server\001");
}

void GameSession::onCharacterLoginResult(const TS_SC_LOGIN_RESULT *packet) {
	handle = packet->handle;
	connectedInGame = true;
	log(LL_Info, "Connected with character %s\n", playername.c_str());

	for(size_t i = 0; i < messageQueue.size(); i++) {
		TS_CS_CHAT_REQUEST* chatRqst = messageQueue.at(i);
		sendPacket(chatRqst);
		TS_MESSAGE_WNA::destroy(chatRqst);
	}
	messageQueue.clear();
}

void GameSession::onEnter(const TS_SC_ENTER *packet) {
	if(packet->objType == EOT_Player) {
		playerNames[packet->handle] = packet->playerInfo.szName;
	}
}

void GameSession::sendMsgToGS(int type, const char* sender, const char* target, std::string msg) {
	char messageFull[500];
	uint8_t msgLen;


	std::replace(msg.begin(), msg.end(), '\x0D', '\x0A');
	if(msg.size() > 200)
		msg.resize(200, ' ');

	if(sender && sender[0])
		sprintf(messageFull, "%s: %s", sender, msg.c_str());
	else
		sprintf(messageFull, "%s", msg.c_str());

	log(LL_Debug, "[IRC] Msg %d: %s\n", type, messageFull);

	if(sender && sender[0] == '@')
		return;

	if(msg.size() < 1)
		return;

	if(!enableGateway)
		return;

	msgLen = (uint8_t)((strlen(messageFull) > 255) ? 255 : strlen(messageFull));

	TS_CS_CHAT_REQUEST* chatRqst;
	chatRqst = TS_MESSAGE_WNA::create<TS_CS_CHAT_REQUEST, char>(msgLen);

	chatRqst->len = msgLen;
	strcpy(chatRqst->szTarget, target);
	strncpy(chatRqst->message, messageFull, chatRqst->len);
	chatRqst->type = type;
	chatRqst->request_id = 0;

	if(connectedInGame) {
		sendPacket(chatRqst);
		TS_MESSAGE_WNA::destroy(chatRqst);
	} else {
		messageQueue.push_back(chatRqst);
	}
}
