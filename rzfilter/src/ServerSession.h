#ifndef AUTHSESSION_H
#define AUTHSESSION_H

#include "EndpointProxy.h"
#include "IFilterEndpoint.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include <memory>

class ClientSession;
class FilterProxy;
class FilterManager;
class GameClientSessionManager;

/**
 * @brief The ServerSession class
 * Spawned when connecting to a server by the ClientSession class
 */
class ServerSession : public EncryptedSession<PacketSession>, public IFilterEndpoint {
	DECLARE_CLASS(ServerSession)
public:
	ServerSession(bool authMode,
	              ClientSession* clientSession,
	              GameClientSessionManager* gameClientSessionManager,
	              FilterManager* filterManager,
	              FilterManager* converterFilterManager);
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
	int getPacketVersion() { return packetVersion.getAsInt(); }
	void close() { EncryptedSession<PacketSession>::close(); }
	StreamAddress getAddress();
	void banAddress(StreamAddress address);
	bool isStrictForwardEnabled();
	bool isAuthMode() { return authMode; }

	void onClientPacketReceived(const TS_MESSAGE* packet);
	void onClientDisconnected();

	void detachClient();

protected:
	void logPacket(bool outgoing, const TS_MESSAGE* msg);

	virtual void updateObjectName();

	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

private:
	using SocketSession::connect;

	bool authMode;
	ClientSession* clientSession;
	GameClientSessionManager* gameClientSessionManager;

	EndpointProxy clientEndpointProxy;
	std::unique_ptr<FilterProxy> packetFilter;
	std::unique_ptr<FilterProxy> packetConverterFilter;
	IFilterEndpoint* toServerBaseEndpoint;
	IFilterEndpoint* toClientBaseEndpoint;

	std::vector<TS_MESSAGE*> pendingMessages;
};

#endif  // AUTHSESSION_H
