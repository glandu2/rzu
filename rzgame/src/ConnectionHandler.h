#ifndef ICONNECTIONHANDLER_H
#define ICONNECTIONHANDLER_H

#include "Core/Object.h"
#include "Packet/PacketBaseMessage.h"

namespace GameServer {

class ClientSession;

class ConnectionHandler : public Object {
public:
	ConnectionHandler(ClientSession* session) : session(session) {}

	virtual void onPacketReceived(const TS_MESSAGE* packet) = 0;

protected:
	ClientSession* session;
};

}  // namespace GameServer

#endif  // ICONNECTIONHANDLER_H
