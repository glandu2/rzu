#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "Network/RappelzSocket.h"
#include <stdint.h>
#include <unordered_map>
#include <string>

class ClientInfo : public ICallbackGuard
{
public:
	ClientInfo(RappelzSocket *socket);
	~ClientInfo();

	static void startServer();

protected:
	static void onNewConnection(void* instance, ISocket* serverSocket);

	static void onStateChanged(void* instance, ISocket* clientSocket, ISocket::State oldState, ISocket::State newState);
	static void onDataReceived(void* instance, RappelzSocket* clientSocket, const TS_MESSAGE* packet);
	
private:
	static ISocket* serverSocket;
	static std::unordered_map<std::string, ClientInfo*> clients;

	enum State{
		NotLogged,
		Logged,
		ServerSelected,
		ConnectingToGame
	} state;
	char aesKey[32];
	uint32_t server;
	bool useRsaAuth;
	RappelzSocket* socket;
	std::string account;
};

#endif // CLIENTMANAGER_H
