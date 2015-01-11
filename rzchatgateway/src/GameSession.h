#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "PacketSession.h"
#include "ClientGameSession.h"
#include <vector>
#include <unordered_map>

class IrcClient;
struct TS_CS_CHAT_REQUEST;

class GameSession : public ClientGameSession {
public:
	GameSession(const std::string& playername, bool enableGateway, Log* packetLog);

	void setIrcClient(IrcClient* ircClient) { this->ircClient = ircClient; }

	void onGameConnected();
	void onGamePacketReceived(const TS_MESSAGE *packet);
	void onGameDisconnected();

	void setGameServerName(std::string name);
	void sendMsgToGS(int type, const char* sender, const char* target, std::string msg);

protected:
	static void onUpdatePacketExpired(uv_timer_t *timer);

private:
	std::string playername;
	IrcClient* ircClient;
	bool enableGateway;
	bool connectedInGame;

	uv_timer_t updateTimer;

	unsigned int handle;

	std::vector<TS_CS_CHAT_REQUEST*> messageQueue;
	std::unordered_map<unsigned int, std::string> playerNames;
};


#endif // GAMESESSION_H
