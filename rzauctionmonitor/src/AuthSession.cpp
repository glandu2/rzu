#include "AuthSession.h"
#include "Config/ConfigParamVal.h"
#include "Core/EventLoop.h"
#include "GameSession.h"

AuthSession::AuthSession(GameSession* gameSession,
                         cval<std::string>& ip,
                         cval<int>& port,
                         cval<std::string>& serverName,
                         cval<int>& delayTime,
                         const std::string& account,
                         const std::string& password,
                         cval<int>& version)
    : ClientAuthSession(gameSession, version.get()),
      gameSession(gameSession),
      ip(ip),
      port(port),
      account(account),
      password(password),
      serverName(serverName),
      delayTime(delayTime),
      nextDelay(delayTime.get()),
      disconnectRequested(false) {}

void AuthSession::connect() {
	disconnectRequested = false;
	ClientAuthSession::connect(ip, port, account, password);
}

void AuthSession::delayedConnect() {
	int delay = nextDelay;
	if(delay > 0) {
		log(LL_Info, "Will connect to auth in %dms\n", delay);
		delayRecoTimer.start(this, &AuthSession::onDelayRecoExpired, delay, 0);
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

void AuthSession::onAuthDisconnected(bool causedByRemote) {
	if(disconnectRequested == false) {
		if(causedByRemote) {
			log(LL_Error, "Unexpected disconnection from Auth (maybe wrong version ?)\n");
		} else {
			nextDelay = delayTime.get();
		}

		delayedConnect();

		// In case of disconnection by server, increase the reconnection delay in case of repeated kicks
		if(causedByRemote && nextDelay < 600 * 1000) {
			nextDelay *= 2;
		}
	} else {
		log(LL_Info, "Disconnected from auth by request\n");
		if(gameSession->getState() == Stream::UnconnectedState)
			CALLBACK_CALL(disconnectedCallback);
	}
}

void AuthSession::onAuthResult(TS_ResultCode result, const std::string& resultString) {
	if(result != TS_RESULT_SUCCESS) {
		log(LL_Error,
		    "%s: Auth failed result: %d (%s)\n",
		    getAccountName().c_str(),
		    result,
		    resultString.empty() ? "no associated string" : resultString.c_str());
		abortSession();
	} else {
		log(LL_Debug, "Retrieving server list\n");
		retreiveServerList();
	}
}

void AuthSession::onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId) {
	bool serverFound = false;
	int serverIndex = 0;
	std::string serverNameToSelect = serverName.get();

	log(LL_Debug, "Server list (last id: %d)\n", lastSelectedServerId);
	for(size_t i = 0; i < servers.size(); i++) {
		const ServerInfo& serverInfo = servers.at(i);
		log(LL_Debug,
		    "%d: %20s at %16s:%d %d%% user ratio\n",
		    serverInfo.serverId,
		    serverInfo.serverName.c_str(),
		    serverInfo.serverIp.c_str(),
		    serverInfo.serverPort,
		    serverInfo.userRatio);

		if(serverInfo.serverName == serverNameToSelect && !serverFound) {
			serverFound = true;
			serverIndex = serverInfo.serverId;
		}
	}

	if(!serverFound) {
		log(LL_Error, "Server \"%s\" not found\n", serverNameToSelect.c_str());
		log(LL_Info, "Server list (last id: %d)\n", lastSelectedServerId);
		for(size_t i = 0; i < servers.size(); i++) {
			const ServerInfo& serverInfo = servers.at(i);
			log(LL_Info,
			    "%d: %20s at %16s:%d %d%% user ratio\n",
			    serverInfo.serverId,
			    serverInfo.serverName.c_str(),
			    serverInfo.serverIp.c_str(),
			    serverInfo.serverPort,
			    serverInfo.userRatio);
		}
		closeSession();
	} else {
		log(LL_Info, "Connecting to GS %s with index %d\n", serverNameToSelect.c_str(), serverIndex);
		selectServer(serverIndex);
		connectedToGS = false;
	}
}

void AuthSession::onGameDisconnected(bool causedByRemote) {
	if(disconnectRequested == false) {
		if(causedByRemote) {
			log(LL_Error, "Unexpected disconnection from GS\n");
		} else {
			nextDelay = delayTime.get();
		}

		delayedConnect();

		// In case of disconnection by server, increase the reconnection delay in case of repeated kicks
		if(causedByRemote && nextDelay < 600 * 1000) {
			nextDelay *= 2;
		}
	} else {
		log(LL_Info, "Disconnected from GS by request\n");
		if(getState() == Stream::UnconnectedState)
			CALLBACK_CALL(disconnectedCallback);
	}
}

void AuthSession::onGameResult(TS_ResultCode result) {
	if(connectedToGS)
		return;
	if(result != TS_RESULT_SUCCESS) {
		log(LL_Error, "GS returned an error while authenticating: %d\n", result);
		abortSession();
	} else {
		log(LL_Info, "Connected to GS\n");
		connectedToGS = true;
	}
}
