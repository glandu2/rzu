#define __STDC_LIMIT_MACROS
#include "GameServerSession.h"
#include <string.h>
#include "../GlobalConfig.h"
#include "Core/PrintfFormats.h"
#include "AuthSession.h"

#include "PacketEnums.h"
#include "AuthGame/TS_AG_LOGIN_RESULT.h"
#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "AuthGame/TS_AG_KICK_CLIENT.h"
#include "AuthGame/TS_AG_ITEM_PURCHASED.h"

namespace AuthServer {

GameServerSession::GameServerSession()
  : authSession(nullptr), serverIdx(UINT16_MAX)
{
}

GameServerSession::~GameServerSession() {
	if(authSession)
		authSession->forceClose();
}

void GameServerSession::onConnected() {
}

void GameServerSession::onDisconnected(bool causedByRemote) {
	if(authSession) {
		if(causedByRemote) {
			info("GS disconnected, disconnecting auth session\n");
			authSession->disconnect();
		} else if(authSession) {
			authSession->forceClose();
		}
	}

	authSession = nullptr;
}

void GameServerSession::disconnectAuth() {
	if(authSession) {
		authSession->disconnect();
		authSession = nullptr;
	}
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

		default:
			if(authSession) {
				debug("Received packet id %d from GS, forwarding to auth\n", packet->id);
				authSession->sendPacket(packet);
			}
			break;
	}
}

void GameServerSession::onServerLogin(const TS_GA_LOGIN* packet) {
	//to manage non null terminated strings
	std::string localServerName, localServerIp, localScreenshotUrl;

	localServerName = Utils::convertToString(packet->server_name, sizeof(packet->server_name)-1);
	localServerIp = Utils::convertToString(packet->server_ip, sizeof(packet->server_ip)-1);
	localScreenshotUrl = Utils::convertToString(packet->server_screenshot_url, sizeof(packet->server_screenshot_url)-1);

	info("Server Login: %s[%d] at %s:%d\n", localServerName.c_str(), packet->server_idx, localServerIp.c_str(), packet->server_port);

	if(authSession != nullptr) {
		error("Server %s[%d] already logged on\n",
			  authSession->getServerName().c_str(), authSession->getServerIdx());

		TS_AG_LOGIN_RESULT result;
		TS_MESSAGE::initMessage<TS_AG_LOGIN_RESULT>(&result);
		result.result = TS_RESULT_INVALID_ARGUMENT;
		sendPacket(&result);
		return;
	}

	authSession = new AuthSession(this,
								  packet->server_idx,
								  localServerName,
								  localServerIp,
								  packet->server_port,
								  localScreenshotUrl,
								  packet->is_adult_server);
	authSession->connect();

	serverIdx = packet->server_idx;

	setDirtyObjectName();
}

void GameServerSession::onClientLogin(const TS_GA_CLIENT_LOGIN* packet) {
	if(!authSession) {
		warn("Received client login but GS not logged on\n");
		return;
	}

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
	if(!authSession) {
		warn("Received client logout but GS not logged on\n");
		return;
	}

	authSession->logoutClient(static_cast<const TS_GA_CLIENT_LOGOUT*>(packet));
}
/*
void GameServerSession::updateObjectName() {
	if(authSession)
		setObjectName(12 + authSession->getServerName().size(), "ServerInfo[%s]", authSession->getServerName().c_str());
}*/

} // namespace AuthServer
