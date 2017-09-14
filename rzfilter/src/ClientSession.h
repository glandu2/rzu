#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ServerSession.h"
#include "IFilterEndpoint.h"

class FilterProxy;

/**
 * @brief The ClientSession class
 * Spawned when a client connects to the filter
 */
class ClientSession : public EncryptedSession<PacketSession>, public IFilterEndpoint
{
	DECLARE_CLASS(ClientSession)

public:
	ClientSession(bool authMode);

	void sendPacket(MessageBuffer& buffer) {
		if(buffer.checkPacketFinalSize() == false) {
			log(LL_Error, "Wrong packet buffer size, id: %d, size: %d, field: %s\n", buffer.getMessageId(), buffer.getSize(), buffer.getFieldInOverflow().c_str());
		} else {
			logPacket(true, (const TS_MESSAGE*)buffer.getData());
			write(buffer.getWriteRequest());
		}
	}
	void sendPacket(const TS_MESSAGE *data) { PacketSession::sendPacket(data); }
	int getPacketVersion() { return version; }

	virtual void onServerPacketReceived(const TS_MESSAGE* packet);

	const char* getPacketName(int16_t id);

protected:
	void logPacket(bool outgoing, const TS_MESSAGE* msg);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);

	virtual std::string getServerIp() = 0;
	virtual uint16_t getServerPort() = 0;

	virtual void updateObjectName();

	~ClientSession();

private:

	ServerSession serverSession;
	FilterProxy* packetFilter;
	int version;
	bool authMode;
};

#endif // CLIENTSESSION_H
