#ifndef CHATAUTHSESSION_H
#define CHATAUTHSESSION_H

#include "Core/Timer.h"
#include "NetSession/ClientAuthSession.h"
#include <string>

class GameSession;

class ChatAuthSession : public ClientAuthSession {
public:
	ChatAuthSession(GameSession* gameSession,
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

	virtual void onAuthDisconnected(bool causedByRemote);
	virtual void onAuthResult(TS_ResultCode result, const std::string& resultString);
	virtual void onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId);

	virtual void onGameDisconnected(bool causedByRemote);
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

	Timer<ChatAuthSession> delayRecoTimer;
	Timer<ChatAuthSession> ggRecoTimer;
};

#endif  // CHATAUTHSESSION_H
