#include "AuthServerSession.h"
#include "ClientSession.h"
#include "Config/ConfigInfo.h"
#include "Core/Utils.h"
#include <string.h>

#include "AuthGame/TS_GA_CLIENT_LOGIN.h"
#include "AuthGame/TS_GA_CLIENT_LOGOUT.h"
#include "AuthGame/TS_GA_LOGIN.h"
#include "PacketEnums.h"

namespace GameServer {

AuthServerSession* AuthServerSession::instance = nullptr;

struct AuthServerSessionConfig {
	cval<bool>& isAdultServer;
	cval<std::string>&name, &screenshotUrl, &listenIp;
	cval<int>&listenPort, &serverIdx;

	cval<std::string>& authIp;
	cval<int>& authPort;
	cval<bool>& authAutoConnect;

	AuthServerSessionConfig()
	    : isAdultServer(CFG_CREATE("game.isadultserver", false)),
	      name(CFG_CREATE("game.name", "GS Emu")),
	      screenshotUrl(CFG_CREATE("game.screenshoturl", "about:blank")),
	      listenIp(CFG_CREATE("game.listen.externalip", "127.0.0.1")),
	      listenPort(CFG_CREATE("game.listen.port", 4514)),
	      serverIdx(CFG_CREATE("game.serveridx", 1)),
	      authIp(CFG_CREATE("auth.ip", "127.0.0.1")),
	      authPort(CFG_CREATE("auth.port", 4502)),
	      authAutoConnect(CFG_CREATE("auth.autoconnect", true)) {}
};

static AuthServerSessionConfig* config = nullptr;

void AuthServerSession::init() {
	config = new AuthServerSessionConfig();
}

void AuthServerSession::deinit() {
	delete config;
}

AuthServerSession::AuthServerSession()
    : PacketSession(SessionType::AuthGame, SessionPacketOrigin::Client, EPIC_LATEST) {
	instance = this;
}

AuthServerSession::~AuthServerSession() {
	instance = nullptr;
}

void AuthServerSession::loginClient(ClientSession* clientSession,
                                    const std::string& account,
                                    uint64_t oneTimePassword) {
	TS_GA_CLIENT_LOGIN loginMsg;

	if(pendingClients.find(account) != pendingClients.end()) {
		log(LL_Warning, "Client %s already logging in\n", account.c_str());
		return;
	}

	log(LL_Debug, "Login request for account %s\n", account.c_str());

	TS_MESSAGE::initMessage(&loginMsg);

	strcpy(loginMsg.account, account.c_str());
	loginMsg.one_time_key = oneTimePassword;
	sendPacket(&loginMsg);
	pendingClients.insert(std::make_pair(account, clientSession));
}

void AuthServerSession::logoutClient(const char* account, uint32_t playTime) {
	TS_GA_CLIENT_LOGOUT loginMsg;
	TS_MESSAGE::initMessage(&loginMsg);

	strcpy(loginMsg.account, account);
	loginMsg.nContinuousPlayTime = 0;
	sendPacket(&loginMsg);
}

bool AuthServerSession::start() {
	return connect(config->authIp.get().c_str(), config->authPort.get());
}

cval<bool>& AuthServerSession::getAutoStartConfig() {
	return config->authAutoConnect;
}

EventChain<SocketSession> AuthServerSession::onConnected() {
	TS_GA_LOGIN loginMsg;

	TS_MESSAGE::initMessage(&loginMsg);
	loginMsg.server_idx = config->serverIdx;
	strncpy(loginMsg.server_name, config->name.get().c_str(), sizeof(loginMsg.server_name));
	strncpy(
	    loginMsg.server_screenshot_url, config->screenshotUrl.get().c_str(), sizeof(loginMsg.server_screenshot_url));
	strncpy(loginMsg.server_ip, config->listenIp.get().c_str(), sizeof(loginMsg.server_ip));
	loginMsg.server_port = config->listenPort;
	loginMsg.is_adult_server = config->isAdultServer;

	sendPacket(&loginMsg);

	return PacketSession::onConnected();
}

EventChain<PacketSession> AuthServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_AG_LOGIN_RESULT::packetID:
			onLoginResult(static_cast<const TS_AG_LOGIN_RESULT*>(packet));
			break;

		case TS_AG_CLIENT_LOGIN::packetID:
			onClientLoginResult(static_cast<const TS_AG_CLIENT_LOGIN*>(packet));
			break;
	}

	return PacketSession::onPacketReceived(packet);
}

void AuthServerSession::onLoginResult(const TS_AG_LOGIN_RESULT* packet) {
	if(packet->result != TS_RESULT_SUCCESS)
		log(LL_Error, "Login to auth failed, result: %d\n", packet->result);
	else
		log(LL_Info, "Registered with auth successfully\n");
}

void AuthServerSession::onClientLoginResult(const TS_AG_CLIENT_LOGIN* packet) {
	std::string account = Utils::convertToString(packet->account, sizeof(packet->account) - 1);
	auto it = pendingClients.find(account);
	if(it != pendingClients.end()) {
		ClientSession* client = it->second;
		pendingClients.erase(it);
		client->onAccountLoginResult(packet->result,
		                             account,
		                             packet->nAccountID,
		                             packet->nPCBangUser,
		                             packet->nEventCode,
		                             packet->nAge,
		                             packet->nContinuousPlayTime,
		                             packet->nContinuousLogoutTime);
	}
}

}  // namespace GameServer
