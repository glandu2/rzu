#pragma once

#include "AuthGame/TS_GA_ACCOUNT_LIST.h"
#include "Core/Timer.h"
#include "NetSession/PacketSession.h"
#include <string>
#include <time.h>
#include <vector>

struct TS_AG_LOGIN_RESULT;
struct TS_GA_CLIENT_LOGOUT;
struct TS_AG_CLIENT_LOGIN_EXTENDED;
class IWritableConsole;

namespace AuthServer {

class GameServerSession;

class AuthSession : public PacketSession {
	DECLARE_CLASS(AuthServer::AuthSession)
public:
	static void init();

	AuthSession(GameServerSession* gameServerSession,
	            uint16_t serverIdx,
	            std::string serverName,
	            std::string serverIp,
	            int32_t serverPort,
	            std::string serverScreenshotUrl,
	            bool isAdultServer);
	~AuthSession();

	const std::list<TS_GA_ACCOUNT_LIST::AccountInfo>& getAccountList() { return accountList; }

	void connect();
	void disconnect();
	void forceClose();
	bool isConnected() { return getStream() && getStream()->getState() == Stream::ConnectedState; }
	bool isServerLoggedOn() { return synchronizedWithAuth; }
	bool isSynchronizedWithAuth() { return synchronizedWithAuth; }

	bool loginServer();
	void logoutClient(const TS_GA_CLIENT_LOGOUT* packet);

	void sendPacket(const TS_MESSAGE* message);

	uint16_t getServerIdx() { return serverIdx; }
	std::string getServerName() { return serverName; }
	std::string getServerIp() { return serverIp; }
	int32_t getServerPort() { return serverPort; }
	std::string getServerScreenshotUrl() { return serverScreenshotUrl; }
	bool getIsAdultServer() { return isAdultServer; }
	time_t getCreationTime() { return creationTime; }

protected:
	void sendLogin();
	void sendAccountList();

	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

	void onTimerReconnect();
	void sendPacketToNetwork(const TS_MESSAGE* message);
	// void updateObjectName();

	void onLoginResult(const TS_AG_LOGIN_RESULT* packet);
	void onClientLoginResult(const TS_AG_CLIENT_LOGIN_EXTENDED* packet);

	void sendPendingMessages();

	static void commandList(IWritableConsole* console, const std::vector<std::string>& args);

private:
	using SocketSession::connect;

	static std::unordered_map<uint16_t, AuthSession*> servers;

	GameServerSession* gameServerSession;

	uint16_t serverIdx;
	std::string serverName;
	std::string serverIp;
	int32_t serverPort;
	std::string serverScreenshotUrl;
	bool isAdultServer;
	time_t creationTime;
	uint8_t guid[16];  // Used to detect lost network connection on the auth side. Must be non predictible

	bool sentLoginPacket;
	bool pendingLogin;
	bool synchronizedWithAuth;
	Timer<AuthSession> recoTimer;

	std::vector<TS_MESSAGE*> pendingMessages;
	std::list<TS_GA_ACCOUNT_LIST::AccountInfo> accountList;
};

}  // namespace AuthServer

