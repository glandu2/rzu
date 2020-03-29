#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "Core/Timer.h"
#include "NetSession/AutoClientSession.h"
#include "NetSession/PacketSession.h"
#include "Packet/GameTypes.h"
#include <unordered_map>
#include <vector>

class IrcClient;
struct TS_CS_CHAT_REQUEST;
struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_ENTER;
struct TS_SC_CHAT_LOCAL;
struct TS_SC_CHAT;
struct TS_TIMESYNC;
struct TS_SC_GAME_TIME;

class GameSession : public AutoClientSession {
public:
	GameSession(bool enableGateway,
	            const std::string& ip,
	            uint16_t port,
	            const std::string& account,
	            const std::string& password,
	            int serverIdx,
	            const std::string& playername,
	            int epic,
	            int delayTime = 5000,
	            int ggRecoTime = 280);

	void setIrcClient(IrcClient* ircClient) { this->ircClient = ircClient; }

	virtual void onConnected(EventTag<AutoClientSession>) override final;
	virtual void onPacketReceived(const TS_MESSAGE* packet, EventTag<AutoClientSession>) override final;
	virtual void onDisconnected(EventTag<AutoClientSession>) override final;

	void setGameServerName(std::string name);
	void sendMsgToGS(int type, const char* sender, const char* target, std::string msg);

protected:
	void onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet);
	void onEnter(const TS_SC_ENTER* packet);
	void onChatLocal(const TS_SC_CHAT_LOCAL* packet);
	void onChat(const TS_SC_CHAT* packet);

private:
	IrcClient* ircClient;
	bool enableGateway;

	std::vector<TS_CS_CHAT_REQUEST> messageQueue;
	std::unordered_map<unsigned int, std::string> playerNames;
};

#endif  // GAMESESSION_H
