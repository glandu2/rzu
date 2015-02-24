#ifndef AUTHSESSION_H
#define AUTHSESSION_H

#include "PacketSession.h"
#include "Packets/TS_GA_CLIENT_LOGGED_LIST.h"

namespace AuthServer {

class GameServerSession;

class AuthSession : public PacketSession
{
	DECLARE_CLASS(AuthServer::AuthSession)
public:
	AuthSession(GameServerSession* gameServerSession);
	~AuthSession();

	static const std::unordered_map<uint16_t, AuthSession*>& getServerList() { return servers; }
	const std::list<TS_GA_CLIENT_LOGGED_LIST::AccountInfo>& getAccountList() { return accountList; }

	void connect();
	void disconnect();
	void forceClose();
	bool isConnected() { return getStream() && getStream()->getState() == Stream::ConnectedState; }
	bool isServerLoggedOn() { return synchronizedWithAuth; }
	bool isSynchronizedWithAuth() { return synchronizedWithAuth; }

	void onConnected();
	void onDisconnected(bool causedByRemote);

	bool loginServer(uint16_t serverIdx,
					 std::string serverName,
					 std::string serverIp,
					 int32_t serverPort,
					 std::string serverScreenshotUrl,
					 bool isAdultServer);
	void logoutClient(const char* account);

	void sendPacket(const TS_MESSAGE* message);

	uint16_t getServerIdx() { return serverIdx; }
	std::string getServerName() { return serverName; }
	std::string getServerIp() { return serverIp; }
	int32_t getServerPort() { return serverPort; }
	std::string getServerScreenshotUrl() { return serverScreenshotUrl; }
	bool getIsAdultServer() { return isAdultServer; }

protected:
	void sendLogin();
	void sendAccountList();
	void onPacketReceived(const TS_MESSAGE* packet);
	static void onTimerReconnect(uv_timer_t* timer);
	void sendPacketToNetwork(const TS_MESSAGE* message);
	void updateObjectName();

private:
	static std::unordered_map<uint16_t, AuthSession*> servers;

	GameServerSession* gameServerSession;

	uint16_t serverIdx;
	std::string serverName;
	std::string serverIp;
	int32_t serverPort;
	std::string serverScreenshotUrl;
	bool isAdultServer;

	bool disconnectRequested;
	bool sentLoginPacket;
	bool pendingLogin;
	bool serverInfoValid;
	bool synchronizedWithAuth;
	uint64_t uniqueIdentifier;
	uv_timer_t* recoTimer;

	std::vector<TS_MESSAGE*> pendingMessages;
	std::list<TS_GA_CLIENT_LOGGED_LIST::AccountInfo> accountList;
};

} // namespace AuthServer

#endif // AUTHSESSION_H
