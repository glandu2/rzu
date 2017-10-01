#include "TestPacketServer.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include "TestPacketSession.h"

TestPacketServer::TestPacketServer(TestConnectionChannel* channel,
                                   cval<std::string>& host,
                                   cval<int>& port,
                                   bool encryptedConnection,
                                   Log* packetLogger)
    : SessionServerCommon(host, port, nullptr, packetLogger),
      channel(channel),
      encryptedConnection(encryptedConnection) {}

void TestPacketServer::updateObjectName() {
	setObjectName(37, "TestPacketServer<TestPacketSession>");
}

SocketSession* TestPacketServer::createSession() {
	if(encryptedConnection)
		return new TestPacketSession<EncryptedSession<PacketSession>>(channel);
	else
		return new TestPacketSession<PacketSession>(channel);
}

bool TestPacketServer::hasCustomPacketLogger() {
	return PacketSession::hasCustomPacketLoggerStatic();
}
