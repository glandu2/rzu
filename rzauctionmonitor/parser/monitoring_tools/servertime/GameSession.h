#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "NetSession/PacketSession.h"
#include "NetSession/ClientGameSession.h"
#include <vector>
#include <unordered_map>
#include "Core/Timer.h"
#include "Core/ThreadWork.h"

struct TS_CS_CHAT_REQUEST;
struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_ENTER;
struct TS_TIMESYNC;
struct TS_SC_GAME_TIME;

class GameSession : public ClientGameSession {
public:
	GameSession(const std::string& playername, Log* packetLog);

	void onGameConnected();
	void onGamePacketReceived(const TS_MESSAGE *packet);
	void onGameDisconnected();

protected:
	void onUpdatePacketExpired();
	void onClockExpired();
	void onCheckAuctionExpired();

	void onCharacterList(const TS_SC_CHARACTER_LIST* packet);
	void onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet);
	void onEnter(const TS_SC_ENTER* packet);
	void onTimeSync(const TS_TIMESYNC *packet);
	void onGameTime(const TS_SC_GAME_TIME* packet);

	uint32_t getRappelzTime();

	void waitNextRappelzSecond();

private:
	std::string playername;
	bool connectedInGame;

	uint32_t handle;
	int32_t rappelzTimeOffset;
	int32_t epochTimeOffset;

	Timer<GameSession> updateTimer;
	Timer<GameSession> clockTimer;
	Timer<GameSession> checkAuctionsToBuyTimer;
	ThreadWork<GameSession> auctionQueryWork;

	std::unordered_map<unsigned int, std::string> playerNames;
};


#endif // GAMESESSION_H
