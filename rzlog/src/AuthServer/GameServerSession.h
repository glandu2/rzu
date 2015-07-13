#ifndef AUTHSERVER_GAMESERVERSESSION_H
#define AUTHSERVER_GAMESERVERSESSION_H

#include "PacketSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ClientData.h"
#include <list>

#include "Packets/PacketEnums.h"
#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_ACCOUNT_LIST.h"
#include "Packets/TS_GA_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_KICK_FAILED.h"
#include "Packets/TS_GA_SECURITY_NO_CHECK.h"

struct TS_AG_CLIENT_LOGIN;
struct TS_AG_CLIENT_LOGIN_EXTENDED;

namespace AuthServer {

class GameData;
class DB_SecurityNoCheck;

class GameServerSession : public PacketSession
{
	DECLARE_CLASS(AuthServer::GameServerSession)

public:
	GameServerSession();

	void kickClient(ClientData *clientData);
	void sendNotifyItemPurchased(ClientData* client);

	void setGameData(GameData* gameData);

	void onSecurityNoCheckResult(DB_SecurityNoCheck* securityNoCheckDb, const std::string& account, int32_t mode, bool ok);

protected:
	void onConnected();
	void onPacketReceived(const TS_MESSAGE* packet);

	void onServerLogin(const TS_GA_LOGIN* packet);
	void onAccountList(const TS_GA_ACCOUNT_LIST *packet);
	void onServerLogout(const TS_GA_LOGOUT* packet);
	void onClientLogin(const TS_GA_CLIENT_LOGIN* packet);
	void onClientLogout(const TS_GA_CLIENT_LOGOUT* packet);
	void onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet);
	void onSecurityNoCheck(const TS_GA_SECURITY_NO_CHECK* packet);

private:
	void fillClientLoginResult(TS_AG_CLIENT_LOGIN* packet, const char* account, TS_ResultCode result, ClientData* clientData);
	void fillClientLoginExtendedResult(TS_AG_CLIENT_LOGIN_EXTENDED* packet, const char* account, TS_ResultCode result, ClientData* clientData);
	void sendClientLoginResult(const char *account, TS_ResultCode result, ClientData* clientData);

private:
	~GameServerSession();

	GameData* gameData;
	bool useAutoReconnectFeature;
	bool securityNoSendMode; //if true, send mode in security no reply (with e6+)

	std::vector<TS_GA_ACCOUNT_LIST::AccountInfo> alreadyConnectedAccounts;
	std::list<DB_SecurityNoCheck*> securityNoCheckQueries;
};

} // namespace AuthServer

#endif // AUTHSERVER_GAMESERVERSESSION_H
