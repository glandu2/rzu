#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "NetSession/PacketSession.h"
#include "NetSession/ClientGameSession.h"
#include <vector>
#include <unordered_map>
#include "Core/Timer.h"

struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
class AuctionWorker;

class GameSession : public ClientGameSession {
	DECLARE_CLASS(GameSession)
public:
	GameSession(AuctionWorker* auctionWorker, const std::string& playername, int ggRecoTime);

	void onGameConnected();
	void onGamePacketReceived(const TS_MESSAGE *packet);
	void onGameDisconnected();

	void auctionSearch(int category_id, int page);

protected:
	void onUpdatePacketExpired();
	void onGGPreventionTimerExpired();
	void onGGTimerExpired();

	void onCharacterList(const TS_SC_CHARACTER_LIST* packet);
	void onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet);

	void setConnected(bool connected);
	uint32_t getRappelzTime();

private:
	AuctionWorker *auctionWorker;
	std::string playername;
	bool connectedInGame;
	bool shouldReconnect;

	uint32_t handle;
	int32_t rappelzTimeOffset;
	int32_t epochTimeOffset;

	Timer<GameSession> updateTimer;

	int ggRecoTime;
	Timer<GameSession> ggPreventionRecoTimer; //allow graceful reconnect when no auction search is in progress
	Timer<GameSession> ggRecoTimer;
};


#endif // GAMESESSION_H
