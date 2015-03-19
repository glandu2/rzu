#include "RzTest.h"
#include "Packets/PacketBaseMessage.h"
#include "EventLoop.h"
#include "TestPacketSession.h"
#include "TestPacketServer.h"
#include "EncryptedSession.h"
#include <tuple>

RzTest::RzTest()
{
}

void RzTest::addChannel(TestConnectionChannel* channel) {
	channels.push_back(channel);
	channel->start();
}
/*
void RzTest::addClientConnection(std::string name, std::string testedExecutableConfig, bool encryptedSession) {
	char* arg = new char[64];
	std::string hostName = TestPacketServer::getHostname(name);
	sprintf(arg, "/%s=%s", testedExecutableConfig.c_str(), hostName.c_str());
	testedExecArgs.push_back(arg);

	PacketSession* session;
	if(encryptedSession)
		session = new TestPacketSession<EncryptedSession<PacketSession>>(name, this);
	else
		session = new TestPacketSession<PacketSession>(name, this);

	sessions.insert(std::make_pair(name, session));
	managedClients.push_back(session);

	session->connect(hostName.c_str(), 0);
}

void RzTest::addServerListener(std::string name, std::string testedExecutableConfig, bool encryptedSession) {
	char* arg = new char[64];
	std::string hostName = TestPacketServer::getHostname(name);
	sprintf(arg, "/%s=%s", testedExecutableConfig.c_str(), hostName.c_str());
	testedExecArgs.push_back(arg);

	TestPacketServer* server = new TestPacketServer(this, name, encryptedSession);
	managedServer.push_back(server);

	server->start();
}*/

void RzTest::run() {
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}
