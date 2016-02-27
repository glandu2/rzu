#ifndef AUTHSESSION_H
#define AUTHSESSION_H

#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include "IFilterEndpoint.h"

class ClientSession;

class ServerSession : public EncryptedSession<PacketSession>, public IFilterEndpoint
{
	DECLARE_CLASS(ServerSession)
public:
	ServerSession(ClientSession* clientSession);
	~ServerSession();

	void connect();

	void sendPacket(const TS_MESSAGE* message);

	void sendPacket(MessageBuffer& buffer) {
		if(buffer.checkFinalSize() == false) {
			log(LL_Error, "Wrong packet buffer size, id: %d, size: %d, field: %s\n", buffer.getMessageId(), buffer.getSize(), buffer.getFieldInOverflow().c_str());
		} else {
			sendPacket((const TS_MESSAGE*)buffer.getData());
		}
	}
	int getPacketVersion() { return version; }

protected:
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

private:
	using SocketSession::connect;

	ClientSession* clientSession;

	std::vector<TS_MESSAGE*> pendingMessages;
	int version;
};

#endif // AUTHSESSION_H
