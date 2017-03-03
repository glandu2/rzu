#include "ChatAuthSession.h"
#include "GameSession.h"
#include "Core/EventLoop.h"
#include "Packet/PacketEpics.h"

ChatAuthSession::ChatAuthSession(GameSession* gameSession, const std::string& ip, uint16_t port, const std::string& account, const std::string& password, int serverIdx, int delayTime, int ggRecoTime, AuthCipherMethod method)
    : ClientAuthSession(gameSession, EPIC_LATEST), gameSession(gameSession), ip(ip), port(port), account(account), password(password), serverIdx(serverIdx), method(method), delayTime(delayTime), ggRecoTime(ggRecoTime)
{
}

void ChatAuthSession::connect() {
	ClientAuthSession::connect(ip, port, account, password, method);
}

void ChatAuthSession::delayedConnect() {
	if(delayTime > 0) {
		log(LL_Info, "Will connect to auth in %dms\n", delayTime);
		delayRecoTimer.start(this, &ChatAuthSession::onDelayRecoExpired, delayTime, 0);
	} else {
		connect();
	}
}

void ChatAuthSession::onDelayRecoExpired() {
	log(LL_Info, "End of delay connect timer, connecting to auth now\n");
	connect();
}

void ChatAuthSession::onAuthDisconnected() {
	delayedConnect();
}

void ChatAuthSession::onAuthResult(TS_ResultCode result, const std::string& resultString) {
	if(result != TS_RESULT_SUCCESS) {
		log(LL_Error, "%s: Auth failed result: %d (%s)\n", account.c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
		abortSession();
	} else {
		log(LL_Info, "Retrieving server list\n");
		retreiveServerList();
	}
}

void ChatAuthSession::onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId) {
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
			gameSession->setGameServerName(serverInfo.serverName);
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

	if(ggRecoTime > 0) {
		log(LL_Debug, "Starting GG timer: %ds\n", ggRecoTime);
		ggRecoTimer.start(this, &ChatAuthSession::onGGTimerExpired, ggRecoTime*1000, 0);
	}
}

void ChatAuthSession::onGameDisconnected() {
	delayedConnect();
}

void ChatAuthSession::onGGTimerExpired() {
	log(LL_Info, "GG timeout, reconnecting\n");
	gameSession->abortSession();
}

void ChatAuthSession::onGameResult(TS_ResultCode result) {
	if(result != TS_RESULT_SUCCESS) {
		log(LL_Error, "GS returned an error while authenticating: %d\n", result);
		abortSession();
	} else {
		log(LL_Info, "Connected to GS\n");
	}
}
