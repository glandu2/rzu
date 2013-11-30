#ifndef PLAYERCOUNTMONITOR_H
#define PLAYERCOUNTMONITOR_H

#include <QObject>
#include "Network/RappelzSocket.h"
#include "Interfaces/ICallbackGuard.h"
#include <QTimer>
#include <QCoreApplication>

class PlayerCountMonitor : public QObject, private ICallbackGuard
{
	Q_OBJECT

	public:
		PlayerCountMonitor(std::string host, uint16_t port, int intervalms = 3500);

		inline uint32_t getPlayerNumber() { return playerNumber; }
		inline void waitFirstUpdate() { while(playerNumber == -1) QCoreApplication::processEvents(); }
		inline void start() { updatePlayerNumber(); timer.start(); }
		inline void stop() { timer.stop(); }
		inline bool isActive() { return timer.isActive(); }

	protected slots:
		void updatePlayerNumber();

	protected:
		static void onPlayerCountReceived(void* instance, RappelzSocket *sock, const TS_MESSAGE* packetData);
		void playerNumberUpdated();

	private:
		RappelzSocket sock;
		QTimer timer;
		qint32 playerNumber;
		quint32 processLoad;
		std::string host;
		uint16_t port;
};

#endif // PLAYERCOUNTMONITOR_H
