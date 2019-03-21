#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "Core/ThreadWork.h"
#include "Core/Timer.h"
#include "NetSession/ClientGameSession.h"
#include "NetSession/PacketSession.h"
#include <unordered_map>
#include <vector>

struct TS_CS_CHAT_REQUEST;
struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_ENTER;
struct TS_TIMESYNC;
struct TS_SC_GAME_TIME;

class GameSession : public ClientGameSession {
public:
	GameSession(const std::string& playername, Log* packetLog, int epic);

	void onGameConnected();
	void onGamePacketReceived(const TS_MESSAGE* packet);
	void onGameDisconnected(bool causedByRemote);

protected:
	void onUpdatePacketExpired();
	void onClockExpired();
	void onCheckAuctionExpired();

	void onCharacterList(const TS_SC_CHARACTER_LIST* packet);
	void onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet);
	void onEnter(const TS_SC_ENTER* packet);
	void onTimeSync(const TS_TIMESYNC* packet);
	void onGameTime(const TS_SC_GAME_TIME* packet);

	uint32_t getGameTime();

	void waitNextGameSecond();

private:
	std::string playername;
	bool connectedInGame;

	uint32_t handle;
	int32_t gameTimeOffset;
	int32_t epochTimeOffset;

	Timer<GameSession> updateTimer;
	Timer<GameSession> clockTimer;
	Timer<GameSession> checkAuctionsToBuyTimer;
	ThreadWork<GameSession> auctionQueryWork;

	std::unordered_map<unsigned int, std::string> playerNames;
};

#endif  // GAMESESSION_H
