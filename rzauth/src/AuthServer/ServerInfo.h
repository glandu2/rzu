#ifndef SERVERINFO_H
#define SERVERINFO_H

#include "Object.h"
#include "RappelzSocket.h"
#include <stdint.h>
#include <vector>
#include <string>
#include "ClientData.h"

#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Packets/TS_GA_CLIENT_KICK_FAILED.h"

namespace AuthServer {

class ServerInfo : public Object, public ICallbackGuard
{
	DECLARE_CLASS(AuthServer::ServerInfo)

public:
	ServerInfo(RappelzSocket* socket);
	~ServerInfo();

	static void startServer();

	static const std::vector<ServerInfo*>& getServerList() { return servers; }

	uint16_t getServerIdx() { return serverIdx; }
	std::string getServerName() { return serverName; }
	std::string getServerIp() { return serverIp; }
	int32_t getServerPort() { return serverPort; }
	std::string getServerScreenshotUrl() { return serverScreenshotUrl; }
	bool getIsAdultServer() { return isAdultServer; }
	void kickClient(const std::string& account);

protected:
	static void onNewConnection(ICallbackGuard* instance, Socket* serverSocket);
	static void onStateChanged(ICallbackGuard* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState);
	static void onDataReceived(ICallbackGuard* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet);

	void onServerLogin(const TS_GA_LOGIN* packet);
	void onClientLogin(const TS_GA_CLIENT_LOGIN* packet);
	void onClientLogout(const TS_GA_CLIENT_LOGOUT* packet);
	void onClientKickFailed(const TS_GA_CLIENT_KICK_FAILED* packet);

private:
	static std::vector<ServerInfo*> servers;

	RappelzSocket* socket;

	uint16_t serverIdx;
	std::string serverName;
	std::string serverIp;
	int32_t serverPort;
	std::string serverScreenshotUrl;
	bool isAdultServer;
};

} // namespace AuthServer

#endif // SERVERINFO_H
