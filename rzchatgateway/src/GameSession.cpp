#include "GameSession.h"
#include "ChatAuthSession.h"
#include "IrcClient.h"
#include "EventLoop.h"
#include <algorithm>

#include "Packets/TS_SC_CHAT.h"
#include "Packets/TS_SC_CHAT_LOCAL.h"
#include "Packets/TS_CS_CHAT_REQUEST.h"
#include "Packets/TS_CS_CHARACTER_LIST.h"
#include "Packets/TS_SC_CHARACTER_LIST.h"
#include "Packets/TS_CS_LOGIN.h"
#include "Packets/TS_SC_LOGIN_RESULT.h"
#include "Packets/TS_TIMESYNC.h"
#include "Packets/TS_SC_ENTER.h"
#include "Packets/TS_SC_DISCONNECT_DESC.h"
#include "Packets/TS_CS_UPDATE.h"

GameSession::GameSession(const std::string& playername, bool enableGateway, Log *packetLog)
	: playername(playername), enableGateway(enableGateway)
{
	ircClient = nullptr;
	handle = 0;
	uv_timer_init(EventLoop::getLoop(), &updateTimer);
	updateTimer.data = this;
	connectedInGame = false;
	//getStream()->setPacketLogger(packetLog);
}

void GameSession::onGameConnected() {
	uv_timer_start(&updateTimer, &onUpdatePacketExpired, 5000, 5000);

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
	uv_timer_stop(&updateTimer);
	connectedInGame = false;
	ircClient->sendMessage("", "\001ACTION disconnected from the game server\001");
}

void GameSession::onUpdatePacketExpired(uv_timer_t *timer) {
	GameSession* thisInstance = (GameSession*)timer->data;

	if(!thisInstance->connectedInGame)
		return;

	TS_CS_UPDATE updatPkt;
	TS_MESSAGE::initMessage<TS_CS_UPDATE>(&updatPkt);

	updatPkt.handle = thisInstance->handle;

	thisInstance->sendPacket(&updatPkt);
}

void GameSession::onGamePacketReceived(const TS_MESSAGE *packet) {
	switch(packet->id) {
		case TS_SC_CHARACTER_LIST::packetID: {
			TS_CS_LOGIN loginPkt;
			TS_TIMESYNC timeSyncPkt;
			const TS_SC_CHARACTER_LIST* charListPkt = reinterpret_cast<const TS_SC_CHARACTER_LIST*>(packet);
			bool characterInList = false;

			TS_MESSAGE::initMessage<TS_CS_LOGIN>(&loginPkt);
			TS_MESSAGE::initMessage<TS_TIMESYNC>(&timeSyncPkt);

			debug("Character list: \n");
			for(int i = 0; i < charListPkt->count; i++) {
				debug(" - %s\n", charListPkt->characters[i].name);
				if(!strcmp(playername.c_str(), charListPkt->characters[i].name))
					characterInList = true;
			}

			if(!characterInList) {
				warn("Character \"%s\" not in character list: \n", playername.c_str());
				for(int i = 0; i < charListPkt->count; i++) {
					warn(" - %s\n", charListPkt->characters[i].name);
				}
			}

			strcpy(loginPkt.szName, playername.c_str());
			loginPkt.race = 0;
			sendPacket(&loginPkt);

			timeSyncPkt.time = 0;
			sendPacket(&timeSyncPkt);

			ircClient->sendMessage("", "\001ACTION is connected to the game server\001");

			break;
		}
		case TS_SC_LOGIN_RESULT::packetID: {
				TS_SC_LOGIN_RESULT* loginResultPkt = (TS_SC_LOGIN_RESULT*) packet;
				handle = loginResultPkt->handle;
				connectedInGame = true;
				for(size_t i = 0; i < messageQueue.size(); i++) {
					TS_CS_CHAT_REQUEST* chatRqst = messageQueue.at(i);
					sendPacket(chatRqst);
					TS_MESSAGE_WNA::destroy(chatRqst);
				}
				messageQueue.clear();
				break;
			}
		case TS_SC_ENTER::packetID: {
			TS_SC_ENTER* enterPkt = (TS_SC_ENTER*) packet;
			if(enterPkt->type == 0 && enterPkt->ObjType == 0) {
				TS_SC_ENTER::PlayerInfo* playerInfo = (TS_SC_ENTER::PlayerInfo*) (((char*)enterPkt) + sizeof(TS_SC_ENTER));
				playerNames[enterPkt->handle] = std::string(playerInfo->szName);
			}
			break;
		}
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
	}
}

void GameSession::sendMsgToGS(int type, const char* sender, const char* target, std::string msg) {
	char messageFull[500];
	int msgLen;


	std::replace(msg.begin(), msg.end(), '\x0D', '\x0A');
	if(msg.size() > 200)
		msg.resize(200, ' ');

	if(sender && sender[0])
		sprintf(messageFull, "%s: %s", sender, msg.c_str());
	else
		sprintf(messageFull, "%s", msg.c_str());

	debug("[IRC] Msg %d: %s\n", type, messageFull);

	if(sender[0] == '@')
		return;

	if(msg.size() < 1)
		return;


	if(!enableGateway)
		return;

	msgLen = (strlen(messageFull) > 255) ? 255 : strlen(messageFull);

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
