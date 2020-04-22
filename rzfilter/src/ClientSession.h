#pragma once

#include "IFilterEndpoint.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include "NetSession/SessionServer.h"
#include "ServerSession.h"
#include <memory>
#include <stdint.h>
#include <string>
#include <unordered_map>

class FilterProxy;
class FilterManager;

/**
 * @brief The ClientSession class
 * Spawned when a client connects to the filter
 */
class ClientSession : public EncryptedSession<PacketSession>, public IFilterEndpoint {
	DECLARE_CLASS(ClientSession)

public:
	ClientSession(bool authMode,
	              GameClientSessionManager* gameClientSessionManager,
	              FilterManager* filterManager,
	              FilterManager* converterFilterManager);

	void sendPacket(MessageBuffer& buffer) {
		if(buffer.checkPacketFinalSize() == false) {
			log(LL_Error,
			    "Wrong packet buffer size, id: %d, size: %d, field: %s\n",
			    buffer.getMessageId(),
			    buffer.getSize(),
			    buffer.getFieldInOverflow().c_str());
		} else {
			logPacket(true, (const TS_MESSAGE*) buffer.getData());
			write(buffer.getWriteRequest());
		}
	}
	void sendPacket(const TS_MESSAGE* data) { PacketSession::sendPacket(data); }
	int getPacketVersion() { return packetVersion.getAsInt(); }
	void close() { EncryptedSession<PacketSession>::close(); }
	StreamAddress getAddress();
	void banAddress(StreamAddress address);
	bool isStrictForwardEnabled();

	const char* getPacketName(int16_t id);

	BanManager* getBanManager() { return getServer()->getBanManager(); }

	void logPacket(bool toClient, const TS_MESSAGE* msg);

	void detachServer();

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);

	virtual std::string getServerIp() = 0;
	virtual uint16_t getServerPort() = 0;

	virtual void updateObjectName();

	~ClientSession();

private:
	ServerSession* serverSession;
	bool authMode;
};

