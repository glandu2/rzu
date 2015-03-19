#ifndef TESTPACKETSERVER_H
#define TESTPACKETSERVER_H

#include "SessionServer.h"
#include "TestPacketSession.h"
#include "Extern.h"

class TestConnectionChannel;

class RZTEST_EXTERN TestPacketServer : public SessionServerCommon
{
	DECLARE_CLASS(TestPacketServer)
public:
	TestPacketServer(TestConnectionChannel* channel, cval<std::string>& host, cval<int>& port, bool encryptedConnection, Log* packetLogger = nullptr);

	void updateObjectName();

protected:
	virtual PacketSession* createSession();
	virtual bool hasCustomPacketLogger();

private:
	TestConnectionChannel* channel;
	bool encryptedConnection;
};

#endif // TESTPACKETSERVER_H
