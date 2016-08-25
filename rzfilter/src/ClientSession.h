#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include "ServerSession.h"
#include "IFilterEndpoint.h"

class IFilter;

class ClientSession : public EncryptedSession<PacketSession>, public IFilterEndpoint
{
	DECLARE_CLASS(ClientSession)

public:
	ClientSession();

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

	void onServerPacketReceived(const TS_MESSAGE* packet);

protected:
	void logPacket(bool outgoing, const TS_MESSAGE* msg);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);

	virtual void updateObjectName();

private:
	~ClientSession();

	ServerSession serverSession;
	IFilter* packetFilter;
	int version;
};

#endif // CLIENTSESSION_H
