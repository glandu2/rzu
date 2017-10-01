#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include "NetSession/SocketSession.h"
#include <unordered_map>

class GameSession;

class IrcClient : public SocketSession {
public:
	IrcClient(const std::string& ip,
	          int port,
	          const std::string& hostname,
	          const std::string& channel,
	          const std::string& nickname,
	          Log* packetLog);

	void setGameSession(GameSession* gameSession) { this->gameSession = gameSession; }

	void connect(std::string servername);
	void sendMessage(const char* target, const std::string& msg);
	void sendMsgToIRC(int type, const char* sender, std::string msg);
	bool isConnected() { return joined; }

protected:
	static void parseIrcMessage(const std::string& message,
	                            std::string& prefix,
	                            std::string& command,
	                            std::string& parameters,
	                            std::string& trailing);
	static const char* getChatColor(int type);

private:
	EventChain<SocketSession> onStateChanged(Stream::State oldState, Stream::State newState, bool causedByRemote);
	EventChain<SocketSession> onDataReceived();
	void onIrcLine(const std::string& line);

	using SocketSession::connect;

private:
	GameSession* gameSession;

	std::string servername;
	std::string channelName;
	std::string nickname;
	std::string ip;
	std::string hostname;
	int port;
	bool joined;

	std::vector<char> buffer;
	std::unordered_map<std::string, std::string> mpGsToIrc;
	std::unordered_map<std::string, std::string> mpIrcToGs;
};

#endif  // IRCCLIENT_H
