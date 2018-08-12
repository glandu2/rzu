#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include "IFilterEndpoint.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
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
	ClientSession(bool authMode, FilterManager* filterManager, FilterManager* converterFilterManager);

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
	int getPacketVersion() { return version; }
	void close() { EncryptedSession<PacketSession>::close(); }
	StreamAddress getAddress();
	void banAddress(StreamAddress address);

	virtual void onServerPacketReceived(const TS_MESSAGE* packet);

	const char* getPacketName(int16_t id);

	void logPacket(bool toClient, const TS_MESSAGE* msg);
	bool isAuthMode() { return authMode; }

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);

	virtual std::string getServerIp() = 0;
	virtual uint16_t getServerPort() = 0;

	virtual void updateObjectName();

	~ClientSession();

	int getServerPacketVersion() { return serverSession.getPacketVersion(); }

private:
	ServerSession serverSession;
	std::unique_ptr<FilterProxy> packetFilter;
	std::unique_ptr<FilterProxy> packetConverterFilter;
	int version;
	bool authMode;
};

#endif  // CLIENTSESSION_H
