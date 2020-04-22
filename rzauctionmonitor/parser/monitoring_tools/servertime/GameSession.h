#pragma once

#include "Core/ThreadWork.h"
#include "Core/Timer.h"
#include "NetSession/AutoClientSession.h"
#include "NetSession/PacketSession.h"
#include <unordered_map>
#include <vector>

struct TS_CS_CHAT_REQUEST;
struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_ENTER;
struct TS_TIMESYNC;
struct TS_SC_GAME_TIME;

class GameSession : public AutoClientSession {
public:
	GameSession(const std::string& ip,
	            uint16_t port,
	            const std::string& account,
	            const std::string& password,
	            int serverIdx,
	            const std::string& playername,
	            int epic = EPIC_LATEST,
	            int delayTime = 5000,
	            int ggRecoTime = 280);

protected:
	virtual void onPacketReceived(const TS_MESSAGE* packet, EventTag<AutoClientSession>) override final;

private:
	void onClockExpired();
	void onCheckAuctionExpired();

	void onEnter(const TS_SC_ENTER* packet);
	void onTimeSync(const TS_TIMESYNC* packet);

	void waitNextGameSecond();

private:
	Timer<GameSession> clockTimer;
	Timer<GameSession> checkAuctionsToBuyTimer;
	ThreadWork<GameSession> auctionQueryWork;

	std::unordered_map<unsigned int, std::string> playerNames;
};

