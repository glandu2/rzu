#pragma once

#include "Core/Timer.h"
#include "NetSession/ClientGameSession.h"
#include "NetSession/PacketSession.h"
#include "Packet/GameTypes.h"
#include <unordered_map>
#include <vector>

struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_RESULT;
struct TS_TIMESYNC;
struct TS_SC_GAME_TIME;
class AuctionWorker;

class GameSession : public ClientGameSession {
	DECLARE_CLASS(GameSession)
public:
	GameSession(AuctionWorker* auctionWorker, const std::string& playername, cval<int>& ggRecoTime, cval<int>& version);

	void close();

	void onGameConnected();
	void onGamePacketReceived(const TS_MESSAGE* packet);
	void onGameDisconnected(bool causedByRemote);

	void auctionSearch(int category_id, int page);

protected:
	void onUpdatePacketExpired();
	void onGGPreventionTimerExpired();
	void onGGTimerExpired();

	void onCharacterList(const TS_SC_CHARACTER_LIST* packet);
	void onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet);
	void onResult(const TS_SC_RESULT* resultPacket);
	void onTimeSync(const TS_TIMESYNC* packet);
	void onGameTime(const TS_SC_GAME_TIME* packet);

	void setConnected(bool connected);
	uint32_t getGameTime();

private:
	AuctionWorker* auctionWorker;
	std::string playername;
	bool connectedInGame;
	bool shouldReconnect;

	ar_handle_t handle;
	int32_t gameTimeOffset;
	int32_t epochTimeOffset;

	Timer<GameSession> updateTimer;

	cval<int>& ggRecoTime;
	cval<int>& version;
	Timer<GameSession> ggPreventionRecoTimer;  // allow graceful reconnect when no auction search is in progress
	Timer<GameSession> ggRecoTimer;
};

