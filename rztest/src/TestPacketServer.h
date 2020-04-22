#pragma once

#include "Extern.h"
#include "NetSession/SessionServer.h"

class TestConnectionChannel;

class RZTEST_EXTERN TestPacketServer : public SessionServerCommon {
	DECLARE_CLASS(TestPacketServer)
public:
	TestPacketServer(TestConnectionChannel* channel,
	                 cval<std::string>& host,
	                 cval<int>& port,
	                 bool encryptedConnection,
	                 Log* packetLogger = nullptr);

	void updateObjectName();

protected:
	virtual SocketSession* createSession();
	virtual bool hasCustomPacketLogger();

private:
	TestConnectionChannel* channel;
	bool encryptedConnection;
};

