#ifndef TERMINATOR_H
#define TERMINATOR_H

#include "NetSession/SocketSession.h"
#include "Extern.h"

class RZTEST_EXTERN Terminator : public SocketSession
{
	DECLARE_CLASS(Terminator)
    public:
	    void start(std::string host, uint16_t port, std::string command = "terminate");

	protected:
		EventChain<SocketSession> onConnected();
		EventChain<SocketSession> onDisconnected(bool causedByRemote);
		EventChain<SocketSession> onDataReceived();

    private:
		std::string host;
		uint16_t port;
		std::string command;
};

#endif // TERMINATOR_H
