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

	virtual void sendPacket(MessageBuffer& buffer) override {
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
	virtual void sendPacket(const TS_MESSAGE* data) override { PacketSession::sendPacket(data); }
	virtual int getPacketVersion() override { return packetVersion.getAsInt(); }
	virtual void close() override { EncryptedSession<PacketSession>::close(); }
	virtual StreamAddress getAddress() override;
	virtual void banAddress(StreamAddress address) override;
	virtual bool isStrictForwardEnabled() override;

	BanManager* getBanManager() { return getServer()->getBanManager(); }

	void detachServer();

protected:
	virtual EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet) override;
	virtual EventChain<SocketSession> onConnected() override;
	virtual EventChain<SocketSession> onDisconnected(bool causedByRemote) override;

	virtual std::string getServerIp() = 0;
	virtual uint16_t getServerPort() = 0;

	virtual void updateObjectName() override;

	~ClientSession();

private:
	ServerSession* serverSession;
	bool authMode;
};
