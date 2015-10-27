#ifndef GAMEHANDLER_H
#define GAMEHANDLER_H

#include "../ConnectionHandler.h"
#include "../Model/Character.h"
#include <memory>

namespace GameServer {

class ClientSession;

class GameHandler : public ConnectionHandler
{
	DECLARE_CLASS(GameServer::GameHandler)
public:
	GameHandler(ClientSession* session, std::unique_ptr<Character> character);

	void onPacketReceived(const TS_MESSAGE* packet) override;

private:
	std::unique_ptr<Character> character;
};

} // namespace GameServer

#endif // GAMEHANDLER_H
