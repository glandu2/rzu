#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "Core/Timer.h"
#include "NetSession/ClientGameSession.h"
#include "NetSession/PacketSession.h"
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

class GameSession : public ClientGameSession {
public:
	GameSession(const std::string& playername, bool enableGateway, Log* packetLog);

	void setIrcClient(IrcClient* ircClient) { this->ircClient = ircClient; }

	void onGameConnected();
	void onGamePacketReceived(const TS_MESSAGE* packet);
	void onGameDisconnected();

	void setGameServerName(std::string name);
	void sendMsgToGS(int type, const char* sender, const char* target, std::string msg);

protected:
	void onUpdatePacketExpired();

	void onCharacterList(const TS_SC_CHARACTER_LIST* packet);
	void onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet);
	void onEnter(const TS_SC_ENTER* packet);
	void onChatLocal(const TS_SC_CHAT_LOCAL* packet);
	void onChat(const TS_SC_CHAT* packet);
	void onTimeSync(const TS_TIMESYNC* packet);
	void onGameTime(const TS_SC_GAME_TIME* packet);

	uint32_t getGameTime();

private:
	std::string playername;
	IrcClient* ircClient;
	bool enableGateway;
	bool connectedInGame;

	uint32_t handle;
	int32_t gameTimeOffset;
	int32_t epochTimeOffset;

	Timer<GameSession> updateTimer;

	std::vector<TS_CS_CHAT_REQUEST> messageQueue;
	std::unordered_map<unsigned int, std::string> playerNames;
};

#endif  // GAMESESSION_H
