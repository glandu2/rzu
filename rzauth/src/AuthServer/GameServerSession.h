#ifndef AUTHSERVER_GAMESERVERSESSION_H
#define AUTHSERVER_GAMESERVERSESSION_H

#include "PacketSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGGED_LIST.h"
#include "Packets/TS_GA_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_KICK_FAILED.h"

struct TS_AG_CLIENT_LOGIN;
struct TS_AG_CLIENT_LOGIN_EXTENDED;

namespace AuthServer {

class GameData;

class GameServerSession : public PacketSession
{
	DECLARE_CLASS(AuthServer::GameServerSession)

public:
	GameServerSession();

	void kickClient(ClientData *clientData);
	void sendNotifyItemPurchased(ClientData* client);

	void setGameData(GameData* gameData);

protected:
	void onConnected();
	void onPacketReceived(const TS_MESSAGE* packet);

	void onServerLogin(const TS_GA_LOGIN* packet);
	void onAccountList(const TS_GA_CLIENT_LOGGED_LIST *packet);
	void onServerLogout(const TS_GA_LOGOUT* packet);
	void onClientLogin(const TS_GA_CLIENT_LOGIN* packet);
	void onClientLogout(const TS_GA_CLIENT_LOGOUT* packet);
	void onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet);

private:
	void fillClientLoginResult(TS_AG_CLIENT_LOGIN* packet, const char* account, TS_ResultCode result, ClientData* clientData);
	void fillClientLoginExtendedResult(TS_AG_CLIENT_LOGIN_EXTENDED* packet, const char* account, TS_ResultCode result, ClientData* clientData);
	void sendClientLoginResult(const char* account, TS_ResultCode result, ClientData* clientData);

private:
	~GameServerSession();

	GameData* gameData;
	bool useAutoReconnectFeature;

	std::vector<TS_GA_CLIENT_LOGGED_LIST::AccountInfo> alreadyConnectedAccounts;
};

} // namespace AuthServer

#endif // AUTHSERVER_GAMESERVERSESSION_H
