#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include <string.h>
#include "../GlobalConfig.h"
#include "PrintfFormats.h"
#include "AuthSession.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/TS_AG_KICK_CLIENT.h"
#include "Packets/TS_AG_ITEM_PURCHASED.h"

namespace AuthServer {

GameServerSession::GameServerSession()
  : authSession(nullptr)
{
}

GameServerSession::~GameServerSession() {
}

void GameServerSession::onConnected() {
	authSession = new AuthSession(this);
	authSession->connect();
}

void GameServerSession::onDisconnected(bool causedByRemote) {
	if(causedByRemote) {
		info("GS disconnected, disconnecting auth session\n");
		authSession->disconnect();
	} else if(authSession) {
		authSession->forceClose();
	}

	authSession = nullptr;
}

void GameServerSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_GA_LOGIN::packetID:
			onServerLogin(static_cast<const TS_GA_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGIN::packetID:
			onClientLogin(static_cast<const TS_GA_CLIENT_LOGIN*>(packet));
			break;

		case TS_GA_CLIENT_LOGOUT::packetID:
			onClientLogout(static_cast<const TS_GA_CLIENT_LOGOUT*>(packet));
			break;

		case TS_CC_EVENT::packetID:
			break;

		default:
			debug("Received packet id %d from GS, forwarding to auth\n", packet->id);
			authSession->sendPacket(packet);
			break;
	}
}

void GameServerSession::onServerLogin(const TS_GA_LOGIN* packet) {
	//to manage non null terminated strings
	std::string localServerName, localServerIp, localScreenshotUrl;

	localServerName = Utils::convertToString(packet->server_name, sizeof(packet->server_name));
	localServerIp = Utils::convertToString(packet->server_ip, sizeof(packet->server_ip));
	localScreenshotUrl = Utils::convertToString(packet->server_screenshot_url, sizeof(packet->server_screenshot_url));

	info("Server Login: %s[%d] at %s:%d\n", localServerName.c_str(), packet->server_idx, localServerIp.c_str(), packet->server_port);

	if(!authSession->loginServer(packet->server_idx,
							 localServerName,
							 localServerIp,
							 packet->server_port,
							 localScreenshotUrl,
							 packet->is_adult_server))
	{
		error("Server %s[%d] already logged on, ignoring login message for %s[%d]\n",
			  authSession->getServerName().c_str(), authSession->getServerIdx(), localServerName.c_str(), packet->server_idx);
		return;
	}

	serverIdx = packet->server_idx;

	setDirtyObjectName();
}

void GameServerSession::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	if(!authSession->isSynchronizedWithAuth()) {
		TS_AG_CLIENT_LOGIN result;
		TS_MESSAGE::initMessage<TS_AG_CLIENT_LOGIN>(&result);

		memcpy(result.account, packet->account, sizeof(result.account));
		result.account[sizeof(result.account) - 1] = '\0';

		result.nAccountID = 0;
		result.result = TS_RESULT_ACCESS_DENIED;
		result.nPCBangUser = 0;
		result.nEventCode = 0;
		result.nAge = 0;
		result.nContinuousPlayTime = 0;
		result.nContinuousLogoutTime = 0;

		if(!authSession->isConnected()) {
			warn("Rejecting client %s as auth connection is down\n", result.account);
		} else {
			warn("Client %s login but GS not synchronized with auth\n", result.account);
		}

		sendPacket(&result);
	} else {
		authSession->sendPacket(packet);
	}
}

void GameServerSession::onClientLogout(const TS_GA_CLIENT_LOGOUT* packet) {
	authSession->logoutClient(static_cast<const TS_GA_CLIENT_LOGOUT*>(packet)->account);
	if(authSession->isConnected()) {
		authSession->sendPacket(packet);
	}
}

void GameServerSession::updateObjectName() {
	if(authSession)
		setObjectName(12 + authSession->getServerName().size(), "ServerInfo[%s]", authSession->getServerName().c_str());
}

} // namespace AuthServer
