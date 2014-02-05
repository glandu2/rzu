#ifndef SOCKETSESSION_H
#define SOCKETSESSION_H

#include "Object.h"
#include "Socket.h"
#include <list>

class RappelzServerCommon;

class SocketSession : public Object, public IListener
{
	DECLARE_CLASS(SocketSession)

public:
	virtual void onDataReceived() {}

protected:
	SocketSession();
	SocketSession(Socket* socket);
	virtual ~SocketSession(); //deleted when disconnected by RappelzServer

	Socket* getSocket() { return socket; }

	static void onDataReceived(IListener* instance, Socket* socket);


	void setServer(RappelzServerCommon* server, std::list<Socket*>::iterator socketIterator) { this->server = server; this->socketIterator = socketIterator; }
	friend class RappelzServerCommon;

private:
	Socket* socket;
	RappelzServerCommon* server;
	std::list<Socket*>::iterator socketIterator;
};

#endif // SOCKETSESSION_H
