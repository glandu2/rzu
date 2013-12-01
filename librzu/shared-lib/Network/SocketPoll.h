#ifndef SOCKETPOOL_H
#define SOCKETPOOL_H

#include "Interfaces/ISocketPool.h"
#include <unordered_set>

class ISocket;

class SocketPoll : public CImplement<ISocketPool>
{
public:
	SocketPoll();
	~SocketPoll();

	virtual void IFACECALLCONV addSocket(ISocket* socket);
	virtual void IFACECALLCONV removeSocket(ISocket* socket);

	virtual void IFACECALLCONV run();
	virtual void IFACECALLCONV processEvents(int waitTime);
	virtual void IFACECALLCONV stop();

	virtual void IFACECALLCONV deleteLater(ISocket* socket);

private:
	int pollAbortPipe[2];
	int epollFd;
	bool stopRequested;
	bool running;
	bool updateAcknownledged;
	std::unordered_set<ISocket*> socketsToDelete;
};

#endif // SOCKETPOOL_H
