#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "NetSession/PacketSession.h"
#include "NetSession/ClientGameSession.h"
#include <vector>
#include <unordered_map>

class IrcClient;
struct TS_CS_CHAT_REQUEST;
struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;

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

	void onCharacterList(const TS_SC_CHARACTER_LIST* packet);
	void onCharacterLoginResult(const TS_SC_LOGIN_RESULT* packet);

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
