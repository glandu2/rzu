#define __STDC_LIMIT_MACROS
#include "AuthServerSession.h"
#include <string.h>
#include "ConfigInfo.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"

namespace GameServer {

AuthServerSession* AuthServerSession::instance = nullptr;

struct AuthServerSessionConfig {
	cval<bool> &isAdultServer;
	cval<std::string> &name, &screenshotUrl, &listenIp;
	cval<int> &listenPort, &serverIdx;

	cval<std::string> &authIp;
	cval<int> &authPort;
	cval<bool> &authAutoConnect;

	AuthServerSessionConfig() :
		isAdultServer(CFG_CREATE("game.isadultserver", false)),
		name         (CFG_CREATE("game.name" , "GS Emu")),
		screenshotUrl(CFG_CREATE("game.screenshoturl" , "about:blank")),
		listenIp     (CFG_CREATE("game.listen.externalip", "127.0.0.1")),
		listenPort   (CFG_CREATE("game.listen.port", 4514)),
		serverIdx    (CFG_CREATE("game.serveridx", 1)),
		authIp       (CFG_CREATE("auth.ip", "127.0.0.1")),
		authPort     (CFG_CREATE("auth.port", 4502)),
		authAutoConnect(CFG_CREATE("auth.autoconnect", true))
	{}
};

static AuthServerSessionConfig* config = nullptr;

void AuthServerSession::init() {
	config = new AuthServerSessionConfig();
}

void AuthServerSession::deinit() {
	delete config;
}

AuthServerSession::AuthServerSession() {
	instance = this;
}

AuthServerSession::~AuthServerSession() {
	instance = nullptr;
}

void AuthServerSession::loginClient(const char *account) {
	TS_GA_CLIENT_LOGIN loginMsg;
	TS_MESSAGE::initMessage<TS_GA_CLIENT_LOGIN>(&loginMsg);

	strcpy(loginMsg.account, account);
	loginMsg.one_time_key = 0;
	sendPacket(&loginMsg);
}

void AuthServerSession::logoutClient(const char *account) {
	TS_GA_CLIENT_LOGOUT loginMsg;
	TS_MESSAGE::initMessage<TS_GA_CLIENT_LOGOUT>(&loginMsg);

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

void AuthServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_AG_LOGIN_RESULT::packetID:
			onLoginResult(static_cast<const TS_AG_LOGIN_RESULT*>(packet));
			break;
	}
}

void AuthServerSession::onConnected() {
	TS_GA_LOGIN loginMsg;

	TS_MESSAGE::initMessage<TS_GA_LOGIN>(&loginMsg);
	loginMsg.server_idx = config->serverIdx;
	strncpy(loginMsg.server_name, config->name.get().c_str(), sizeof(loginMsg.server_name));
	strncpy(loginMsg.server_screenshot_url, config->screenshotUrl.get().c_str(), sizeof(loginMsg.server_screenshot_url));
	strncpy(loginMsg.server_ip, config->listenIp.get().c_str(), sizeof(loginMsg.server_ip));
	loginMsg.server_port = config->listenPort;
	loginMsg.is_adult_server = config->isAdultServer;

	sendPacket(&loginMsg);
}

void AuthServerSession::onLoginResult(const TS_AG_LOGIN_RESULT* packet) {
	if(packet->result != TS_RESULT_SUCCESS)
		error("Login to auth failed, result: %d\n", packet->result);
	else
		info("Registered with auth successfully\n");
}

} //namespace UploadServer
