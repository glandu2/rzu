#ifndef CHATAUTHSESSION_H
#define CHATAUTHSESSION_H

#include "ClientAuthSession.h"
#include <string>

class GameSession;

class ChatAuthSession : public ClientAuthSession
{
public:
	ChatAuthSession(GameSession* gameSession,
					const std::string& ip,
					uint16_t port,
					const std::string& account,
					const std::string& password,
					int serverIdx,
					int delayTime,
					int ggRecoTime,
					AuthCipherMethod method = ACM_DES);

	void connect();
	void delayedConnect();

private:
	using ClientAuthSession::connect;

	virtual void onAuthDisconnected();
	virtual void onAuthResult(TS_ResultCode result, const std::string& resultString);
	virtual void onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId);

	virtual void onGameDisconnected();
	virtual void onGameResult(TS_ResultCode result);

	static void onDelayRecoExpired(uv_timer_t* timer);
	static void onGGTimerExpired(uv_timer_t* timer);

private:
	GameSession* gameSession;
	std::string ip;
	uint16_t port;
	std::string account;
	std::string password;
	int serverIdx;
	AuthCipherMethod method;

	int delayTime;
	int ggRecoTime;

	uv_timer_t delayRecoTimer;
	uv_timer_t ggRecoTimer;
};

#endif // CHATAUTHSESSION_H
