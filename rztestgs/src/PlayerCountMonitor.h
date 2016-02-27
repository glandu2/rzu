#ifndef PLAYERCOUNTMONITOR_H
#define PLAYERCOUNTMONITOR_H

#include "NetSession/PacketSession.h"
#include "NetSession/EncryptedSession.h"
#include "Core/IListener.h"
#include "uv.h"

class PlayerCountMonitor : public EncryptedSession<PacketSession>
{
	DECLARE_CLASSNAME(PlayerCountMonitor, 0)
	public:
		PlayerCountMonitor(std::string host, uint16_t port, const std::string& reqStr, int intervalms = 3500);
		void start();
		void stop();

	protected:
		static void updatePlayerNumberStatic(uv_timer_t* handle);

	protected:
		EventChain<SocketSession> onConnected();
		EventChain<SocketSession> onDisconnected(bool causedByRemote);
		EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);
		void updatePlayerNumber();

	private:
		uv_timer_t timer;
		std::string host;
		uint16_t port;
		int timeout;
		std::string reqStr;

		int connectedTimes;
};

#endif // PLAYERCOUNTMONITOR_H
