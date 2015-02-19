#ifndef AUTHSESSION_H
#define AUTHSESSION_H

#include "PacketSession.h"

namespace AuthServer {

class GameServerSession;

class AuthSession : public PacketSession
{
	DECLARE_CLASS(AuthServer::AuthSession)
public:
	AuthSession(GameServerSession* gameServerSession,
				uint16_t serverIdx,
				std::string serverName,
				std::string serverIp,
				int32_t serverPort,
				std::string serverScreenshotUrl,
				bool isAdultServer);
	~AuthSession();

	static const std::unordered_map<uint16_t, AuthSession*>& getServerList() { return servers; }

	void connect();
	void disconnect();
	void forceClose();
	bool isConnected() { return getStream() && getStream()->getState() == Stream::ConnectedState; }

	void onConnected();
	void onDisconnected(bool causedByRemote);

	void sendPacket(const TS_MESSAGE* message);

	uint16_t getServerIdx() { return serverIdx; }
	std::string getServerName() { return serverName; }
	std::string getServerIp() { return serverIp; }
	int32_t getServerPort() { return serverPort; }
	std::string getServerScreenshotUrl() { return serverScreenshotUrl; }
	bool getIsAdultServer() { return isAdultServer; }

protected:
	void onPacketReceived(const TS_MESSAGE* packet);
	static void onTimerReconnect(uv_timer_t* timer);

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
	bool loginResultReceived;
	uv_timer_t recoTimer;

	enum State {
		Registering,
		WaitingRegisterResult,
		Registered,
		Unregistering,
		Unregistered
	};

	std::vector<TS_MESSAGE*> pendingMessages;
};

} // namespace AuthServer

#endif // AUTHSESSION_H
