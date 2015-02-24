#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include <string.h>
#include <algorithm>
#include "../GlobalConfig.h"
#include "PrintfFormats.h"

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
			authSession->logoutClient(static_cast<const TS_GA_CLIENT_LOGOUT*>(packet)->account);
			authSession->sendPacket(packet);
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

	localServerName = std::string(packet->server_name, std::find(packet->server_name, packet->server_name + sizeof(packet->server_name), '\0'));
	localServerIp = std::string(packet->server_ip, std::find(packet->server_ip, packet->server_ip + sizeof(packet->server_ip), '\0'));
	localScreenshotUrl = std::string(packet->server_screenshot_url, std::find(packet->server_screenshot_url, packet->server_screenshot_url + sizeof(packet->server_screenshot_url), '\0'));

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

void GameServerSession::updateObjectName() {
	if(authSession)
		setObjectName(12 + authSession->getServerName().size(), "ServerInfo[%s]", authSession->getServerName().c_str());
}

} // namespace AuthServer
