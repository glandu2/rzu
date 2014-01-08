#ifndef PLAYERCOUNTMONITOR_H
#define PLAYERCOUNTMONITOR_H

#include "RappelzSocket.h"
#include "IListener.h"
#include "uv.h"

class PlayerCountMonitor : private IListener
{
	public:
		PlayerCountMonitor(std::string host, uint16_t port, int intervalms = 3500);
		void start();
		void stop();

	protected:
		static void updatePlayerNumber(uv_timer_t* handle, int status);

	protected:
		static void onPlayerCountReceived(IListener* instance, RappelzSocket *sock, const TS_MESSAGE* packetData);

	private:
		RappelzSocket sock;
		uv_timer_t timer;
		std::string host;
		uint16_t port;
		int timeout;
};

#endif // PLAYERCOUNTMONITOR_H
