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

	void sendPacket(const TS_MESSAGE *data) { PacketSession::sendPacket(data); }
	void onServerPacketReceived(const TS_MESSAGE* packet);

protected:
	void logPacket(bool outgoing, const TS_MESSAGE* msg);
	void onPacketReceived(const TS_MESSAGE* packet);
	void onConnected();
	void onDisconnected(bool causedByRemote);

	virtual void updateObjectName();

private:
	~ClientSession();

	ServerSession serverSession;
	IFilter* packetFilter;
};

#endif // CLIENTSESSION_H
