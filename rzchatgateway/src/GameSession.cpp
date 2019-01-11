#include "GameSession.h"
#include "ChatAuthSession.h"
#include "Cipher/RzHashReversible256.h"
#include "IrcClient.h"
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

GameSession::GameSession(const std::string& playername, bool enableGateway, Log* packetLog)
    : ClientGameSession(EPIC_LATEST),
      playername(playername),
      ircClient(nullptr),
      enableGateway(enableGateway),
      connectedInGame(false),
      handle(0),
      gameTimeOffset(0),
      epochTimeOffset(0) {}

void GameSession::onGameConnected() {
	updateTimer.start(this, &GameSession::onUpdatePacketExpired, 5000, 5000);
	epochTimeOffset = gameTimeOffset = 0;

	TS_CS_CHARACTER_LIST charlistPkt;
	charlistPkt.account = auth->getAccountName();
	sendPacket(charlistPkt);
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
	updatPkt.time = getGameTime() + gameTimeOffset;
	updatPkt.epoch_time = uint32_t(time(NULL) + epochTimeOffset);

	sendPacket(updatPkt);
}

uint32_t GameSession::getGameTime() {
	return uint32_t(uv_hrtime() / (10 * 1000 * 1000));
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

		case TS_SC_CHAT_LOCAL::packetID:
			packet->process(this, &GameSession::onChatLocal, version);
			break;

		case TS_SC_CHAT::packetID:
			packet->process(this, &GameSession::onChat, version);
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

	ircClient->sendMessage("", "\001ACTION is connected to the game server\001");
}

void GameSession::onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet) {
	handle = packet->handle;
	connectedInGame = true;
	log(LL_Info, "Connected with character %s\n", playername.c_str());

	for(size_t i = 0; i < messageQueue.size(); i++) {
		const TS_CS_CHAT_REQUEST& chatRqst = messageQueue[i];
		sendPacket(chatRqst);
	}
	messageQueue.clear();
}

void GameSession::onEnter(const TS_SC_ENTER* packet) {
	if(packet->objType == EOT_Player) {
		playerNames[packet->handle] = packet->playerInfo.szName;
	}
}

void GameSession::onChatLocal(const TS_SC_CHAT_LOCAL* packet) {
	std::unordered_map<unsigned int, std::string>::iterator it = playerNames.find(packet->handle);
	std::string playerName = "Unknown";

	if(packet->handle == handle)
		playerName = playername;
	else if(it != playerNames.end())
		playerName = it->second;

	ircClient->sendMsgToIRC(packet->type, playerName.c_str(), packet->message);
}

void GameSession::onChat(const TS_SC_CHAT* packet) {
	ircClient->sendMsgToIRC(packet->type, packet->szSender.c_str(), packet->message);
}

void GameSession::onTimeSync(const TS_TIMESYNC* packet) {
	gameTimeOffset = packet->time - getGameTime();

	TS_TIMESYNC timeSyncPkt;

	timeSyncPkt.time = packet->time;
	sendPacket(timeSyncPkt);

	if(epochTimeOffset == 0) {
		TS_CS_GAME_TIME gameTimePkt;
		sendPacket(gameTimePkt);
	}
}

void GameSession::onGameTime(const TS_SC_GAME_TIME* packet) {
	gameTimeOffset = packet->t - getGameTime();
	epochTimeOffset = int32_t(packet->game_time - time(NULL));
}

void GameSession::sendMsgToGS(int type, const char* sender, const char* target, std::string msg) {
	char messageFull[500];

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

	TS_CS_CHAT_REQUEST chatRqst;

	chatRqst.szTarget = target;
	chatRqst.message = messageFull;
	if(chatRqst.message.size() > 127)
		chatRqst.message.resize(127);

	chatRqst.type = (TS_CHAT_TYPE) type;
	chatRqst.request_id = 0;

	if(connectedInGame) {
		sendPacket(chatRqst);
	} else {
		messageQueue.push_back(chatRqst);
	}
}
