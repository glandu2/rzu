#pragma once

#include "Core/Object.h"

struct TS_MESSAGE;
struct TS_SC_CHAT;

class ConnectionToClient;
class ConnectionToServer;
class MultiServerManager;

class BotHandler : public Object {
	DECLARE_CLASS(BotHandler)
public:
	BotHandler(MultiServerManager* multiServerManager, ConnectionToClient* connectionToClient);

	void onPacketFromClient(const TS_MESSAGE* packet);
	void onPacketFromServer(ConnectionToServer* originatingServer, const TS_MESSAGE* packet);

private:
	void onChat(const TS_SC_CHAT* packet, ConnectionToServer* originatingServer);

private:
	MultiServerManager* multiServerManager;
	ConnectionToClient* connectionToClient;
};