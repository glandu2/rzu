#ifndef CHATAUTHSESSION_H
#define CHATAUTHSESSION_H

#include "NetSession/ClientAuthSession.h"
#include "Core/Timer.h"
#include <string>

class GameSession;

class AuthSession : public ClientAuthSession
{
public:
	typedef void (*DisconnectedCallback)(IListener* instance);
public:
	AuthSession(GameSession* gameSession,
					const std::string& ip,
					uint16_t port,
					const std::string& account,
					const std::string& password,
					int serverIdx,
					int delayTime,
					AuthCipherMethod method = ACM_DES);

	void connect();
	void disconnect(Callback<DisconnectedCallback> callback);
	bool isDisconnectRequested() { return disconnectRequested; }

private:
	using ClientAuthSession::connect;

	void delayedConnect();

	virtual void onAuthDisconnected();
	virtual void onAuthResult(TS_ResultCode result, const std::string& resultString);
	virtual void onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId);

	virtual void onGameDisconnected();
	virtual void onGameResult(TS_ResultCode result);

	void onDelayRecoExpired();

private:
	GameSession* gameSession;
	std::string ip;
	uint16_t port;
	std::string account;
	std::string password;
	int serverIdx;
	AuthCipherMethod method;

	bool disconnectRequested;
	int delayTime;

	Timer<AuthSession> delayRecoTimer;
	Callback<DisconnectedCallback> disconnectedCallback;
};

#endif // CHATAUTHSESSION_H
