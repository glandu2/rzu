#include "TestPacketServer.h"
#include "EncryptedSession.h"

TestPacketServer::TestPacketServer(TestConnectionChannel* channel, const std::string& host, bool encryptedConnection, Log *packetLogger)
	: SessionServerCommon(pipeName, port, nullptr, packetLogger),
	  channel(channel), encryptedConnection(encryptedConnection),
	  pipeName(host), port(0)
{
}

void TestPacketServer::updateObjectName() {
	setObjectName(37, "TestPacketServer<TestPacketSession>");
}

PacketSession *TestPacketServer::createSession() {
	if(encryptedConnection)
		return new TestPacketSession<EncryptedSession<PacketSession>>(channel);
	else
		return new TestPacketSession<PacketSession>(channel);
}

bool TestPacketServer::hasCustomPacketLogger() {
	return PacketSession::hasCustomPacketLoggerStatic();
}
