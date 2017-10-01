#ifndef CHATAUTHSESSION_H
#define CHATAUTHSESSION_H

#include "Core/Timer.h"
#include "NetSession/ClientAuthSession.h"
#include <string>

class GameSession;

class AuthSession : public ClientAuthSession {
public:
	typedef void (*DisconnectedCallback)(IListener* instance);

public:
	AuthSession(GameSession* gameSession,
	            cval<std::string>& ip,
	            cval<int>& port,
	            cval<int>& serverIdx,
	            cval<int>& delayTime,
	            cval<bool>& useRsa,
	            const std::string& account,
	            const std::string& password,
	            cval<int>& version);

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

	cval<std::string>& ip;
	cval<int>& port;
	std::string account;
	std::string password;
	cval<int>& serverIdx;
	cval<int>& delayTime;
	cval<bool>& useRsa;

	bool disconnectRequested;
	bool connectedToGS;

	Timer<AuthSession> delayRecoTimer;
	Callback<DisconnectedCallback> disconnectedCallback;
};

#endif  // CHATAUTHSESSION_H
