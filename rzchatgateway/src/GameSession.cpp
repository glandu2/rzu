#include "GameSession.h"
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

GameSession::GameSession(bool enableGateway,
                         const std::string& ip,
                         uint16_t port,
                         const std::string& account,
                         const std::string& password,
                         const std::string& serverName,
                         const std::string& playername,
                         int epic,
                         int delayTime,
                         int ggRecoTime)
    : AutoClientSession(ip, port, account, password, serverName, playername, epic, true, delayTime, ggRecoTime),
      ircClient(nullptr),
      enableGateway(enableGateway) {}

void GameSession::setGameServerName(std::string name) {
	if(ircClient->isConnected() == false)
		ircClient->connect(name);
}

void GameSession::onDisconnected(EventTag<AutoClientSession>) {
	ircClient->sendMessage("", "\001ACTION disconnected from the game server\001");
}

void GameSession::onPacketReceived(const TS_MESSAGE* packet, EventTag<AutoClientSession>) {
	packet_type_id_t packetType = PacketMetadata::convertPacketIdToTypeId(
	    packet->id, SessionType::GameClient, SessionPacketOrigin::Server, packetVersion);
	switch(packetType) {
		case TS_SC_LOGIN_RESULT::packetID:;
			packet->process(this, &GameSession::onCharacterLoginResult, packetVersion);
			break;

		case TS_SC_ENTER::packetID:
			packet->process(this, &GameSession::onEnter, packetVersion);
			break;

		case TS_SC_CHAT_LOCAL::packetID:
			packet->process(this, &GameSession::onChatLocal, packetVersion);
			break;

		case TS_SC_CHAT::packetID:
			packet->process(this, &GameSession::onChat, packetVersion);
			break;
	}
}

void GameSession::onConnected(EventTag<AutoClientSession> tag) {
	ircClient->sendMessage("", "\001ACTION is connected to the game server\001");
}

void GameSession::onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet) {
	for(size_t i = 0; i < messageQueue.size(); i++) {
		const TS_CS_CHAT_REQUEST& chatRqst = messageQueue[i];
		sendPacket(chatRqst);
	}
	messageQueue.clear();
}

void GameSession::onEnter(const TS_SC_ENTER* packet) {
	if(packet->objType == EOT_Player) {
		playerNames[packet->handle] = packet->playerInfo.name;
	}
}

void GameSession::onChatLocal(const TS_SC_CHAT_LOCAL* packet) {
	std::unordered_map<unsigned int, std::string>::iterator it = playerNames.find(packet->handle);
	std::string playerName;

	if(packet->handle == getLocalPlayerHandle())
		playerName = getPlayerName();
	else if(it != playerNames.end())
		playerName = it->second;
	else
		playerName = "Unknown";

	ircClient->sendMsgToIRC(packet->type, playerName.c_str(), packet->message);
}

void GameSession::onChat(const TS_SC_CHAT* packet) {
	ircClient->sendMsgToIRC(packet->type, packet->szSender.c_str(), packet->message);
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

	if(isConnected()) {
		sendPacket(chatRqst);
	} else {
		messageQueue.push_back(chatRqst);
	}
}
