#ifndef AUTHSESSION_H
#define AUTHSESSION_H

#include "IFilterEndpoint.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"

class ClientSession;

/**
 * @brief The ServerSession class
 * Spawned when connecting to a server by the ClientSession class
 */
class ServerSession : public EncryptedSession<PacketSession>, public IFilterEndpoint {
	DECLARE_CLASS(ServerSession)
public:
	ServerSession(ClientSession* clientSession);
	~ServerSession();

	void connect(std::string ip, uint16_t port);

	void sendPacket(const TS_MESSAGE* message);

	void sendPacket(MessageBuffer& buffer) {
		if(buffer.checkPacketFinalSize() == false) {
			log(LL_Error,
			    "Wrong packet buffer size, id: %d, size: %d, field: %s\n",
			    buffer.getMessageId(),
			    buffer.getSize(),
			    buffer.getFieldInOverflow().c_str());
		} else {
			sendPacket((const TS_MESSAGE*) buffer.getData());
		}
	}
	int getPacketVersion() { return version; }
	void close() { EncryptedSession<PacketSession>::close(); }
	StreamAddress getAddress();
	void banAddress(StreamAddress address);

protected:
	void logPacket(bool outgoing, const TS_MESSAGE* msg);

	virtual void updateObjectName();

	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

private:
	using SocketSession::connect;

	ClientSession* clientSession;

	std::vector<TS_MESSAGE*> pendingMessages;
	int version;
};

#endif  // AUTHSESSION_H
