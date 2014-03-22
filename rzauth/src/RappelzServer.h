#ifndef RAPPELZSERVER_H
#define RAPPELZSERVER_H

#include "Object.h"
#include "Socket.h"
#include <list>

class SocketSession;
class BanManager;

class RappelzServerCommon : public Object, public IListener
{
	DECLARE_CLASS(RappelzServerCommon)

public:
	RappelzServerCommon();
	~RappelzServerCommon();

	void startServer(const std::string& interfaceIp, uint16_t port, BanManager* banManager = nullptr);
	void stop();

	Socket::State getState() { return serverSocket->getState(); }

	void socketClosed(std::list<Socket*>::iterator socketIterator) { if(openServer) sockets.erase(socketIterator); }

protected:
	static void onNewConnection(IListener* instance, Socket* serverSocket);
	static void onSocketStateChanged(IListener* instance, Socket*, Socket::State, Socket::State newState);

	virtual SocketSession* createSession() = 0;

private:
	bool openServer;
	Socket* serverSocket;
	SocketSession* lastWaitingInstance;
	std::list<Socket*> sockets;
	BanManager* banManager;
};

template<class T>
class RappelzServer : public RappelzServerCommon
{
protected:
	virtual SocketSession* createSession() {
		return new T();
	}
};

#endif // RAPPELZSERVER_H