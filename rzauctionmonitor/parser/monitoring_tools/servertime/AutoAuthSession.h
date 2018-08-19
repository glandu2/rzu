#ifndef CHATAUTHSESSION_H
#define CHATAUTHSESSION_H

#include "Core/Timer.h"
#include "NetSession/ClientAuthSession.h"
#include <string>

class GameSession;

class AutoAuthSession : public ClientAuthSession {
public:
	AutoAuthSession(GameSession* gameSession,
	                const std::string& ip,
	                uint16_t port,
	                const std::string& account,
	                const std::string& password,
	                int serverIdx,
	                int delayTime,
	                int ggRecoTime,
	                int epic);

	void connect();
	void delayedConnect();

private:
	using ClientAuthSession::connect;

	virtual void onAuthDisconnected();
	virtual void onAuthResult(TS_ResultCode result, const std::string& resultString);
	virtual void onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId);

	virtual void onGameDisconnected();
	virtual void onGameResult(TS_ResultCode result);

	void onDelayRecoExpired();
	void onGGTimerExpired();

private:
	GameSession* gameSession;
	std::string ip;
	uint16_t port;
	std::string account;
	std::string password;
	int serverIdx;

	int delayTime;
	int ggRecoTime;

	Timer<AutoAuthSession> delayRecoTimer;
	Timer<AutoAuthSession> ggRecoTimer;
};

#endif  // CHATAUTHSESSION_H
