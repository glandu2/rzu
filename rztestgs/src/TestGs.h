#ifndef PLAYERCOUNTMONITOR_H
#define PLAYERCOUNTMONITOR_H

#include "Core/IListener.h"
#include "Core/Timer.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include "uv.h"

class TestGs : public EncryptedSession<PacketSession> {
	DECLARE_CLASSNAME(PlayerCountMonitor, 0)
public:
	TestGs(std::string host, uint16_t port, const std::string& reqStr);
	void start();
	void stop();

protected:
	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

private:
	std::string host;
	uint16_t port;
	std::string reqStr;

	int connectedTimes;
};

#endif  // PLAYERCOUNTMONITOR_H
