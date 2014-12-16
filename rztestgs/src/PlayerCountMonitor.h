#ifndef PLAYERCOUNTMONITOR_H
#define PLAYERCOUNTMONITOR_H

#include "PacketSession.h"
#include "EncryptedSession.h"
#include "IListener.h"
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
		void onPacketReceived(const TS_MESSAGE* packet);
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
