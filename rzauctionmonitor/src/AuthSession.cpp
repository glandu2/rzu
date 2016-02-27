#include "AuthSession.h"
#include "GameSession.h"
#include "Core/EventLoop.h"

AuthSession::AuthSession(GameSession* gameSession, const std::string& ip, uint16_t port, const std::string& account, const std::string& password, int serverIdx, int delayTime, AuthCipherMethod method)
	: ClientAuthSession(gameSession),
	  gameSession(gameSession),
	  ip(ip),
	  port(port),
	  account(account),
	  password(password),
	  serverIdx(serverIdx),
	  method(method),
	  disconnectRequested(false),
	  delayTime(delayTime)
{
}

void AuthSession::connect() {
	disconnectRequested = false;
	ClientAuthSession::connect(ip, port, account, password, method);
}

void AuthSession::delayedConnect() {
	if(delayTime > 0) {
		log(LL_Info, "Will connect to auth in %dms\n", delayTime);
		delayRecoTimer.start(this, &AuthSession::onDelayRecoExpired, delayTime, 0);
	} else {
		connect();
	}
}

void AuthSession::disconnect(Callback<DisconnectedCallback> callback) {
	log(LL_Info, "Disconnecting\n");

	disconnectedCallback = callback;
	disconnectRequested = true;
	delayRecoTimer.stop();
	ClientAuthSession::close();
}

void AuthSession::onDelayRecoExpired() {
	log(LL_Info, "End of delay connect timer, connecting to auth now\n");
	connect();
}

void AuthSession::onAuthDisconnected() {
	if(disconnectRequested == false) {
		delayedConnect();
	} else {
		log(LL_Info, "Disconnected from auth by request\n");
		if(gameSession->getState() == Stream::UnconnectedState)
			CALLBACK_CALL(disconnectedCallback);
	}
}

void AuthSession::onAuthResult(TS_ResultCode result, const std::string& resultString) {
	if(result != TS_RESULT_SUCCESS) {
		log(LL_Error, "%s: Auth failed result: %d (%s)\n", account.c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
		abortSession();
	} else {
		log(LL_Debug, "Retrieving server list\n");
		retreiveServerList();
	}
}

void AuthSession::onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId) {
	bool serverFound = false;

	log(LL_Debug, "Server list (last id: %d)\n", lastSelectedServerId);
	for(size_t i = 0; i < servers.size(); i++) {
		const ServerInfo& serverInfo = servers.at(i);
		log(LL_Debug, "%d: %20s at %16s:%d %d%% user ratio\n",
				serverInfo.serverId,
				serverInfo.serverName.c_str(),
				serverInfo.serverIp.c_str(),
				serverInfo.serverPort,
				serverInfo.userRatio);

		if(serverInfo.serverId == serverIdx && !serverFound) {
			serverFound = true;
		}
	}

	if(!serverFound) {
		log(LL_Error, "Server with index %d not found\n", serverIdx);
		log(LL_Info, "Server list (last id: %d)\n", lastSelectedServerId);
		for(size_t i = 0; i < servers.size(); i++) {
			const ServerInfo& serverInfo = servers.at(i);
			log(LL_Info, "%d: %20s at %16s:%d %d%% user ratio\n",
					serverInfo.serverId,
					serverInfo.serverName.c_str(),
					serverInfo.serverIp.c_str(),
					serverInfo.serverPort,
					serverInfo.userRatio);
		}
	}

	log(LL_Info, "Connecting to GS with index %d\n", serverIdx);
	selectServer(serverIdx);
}

void AuthSession::onGameDisconnected() {
	if(disconnectRequested == false) {
		delayedConnect();
	} else {
		log(LL_Info, "Disconnected from GS by request\n");
		if(getState() == Stream::UnconnectedState)
			CALLBACK_CALL(disconnectedCallback);
	}
}

void AuthSession::onGameResult(TS_ResultCode result) {
	if(result != TS_RESULT_SUCCESS) {
		log(LL_Error, "GS returned an error while authenticating: %d\n", result);
		abortSession();
	} else {
		log(LL_Info, "Connected to GS\n");
	}
}
