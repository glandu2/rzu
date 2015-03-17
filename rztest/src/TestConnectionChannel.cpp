#include "TestConnectionChannel.h"
#include "TestPacketServer.h"
#include "TestPacketSession.h"
#include "EncryptedSession.h"

TestConnectionChannel::~TestConnectionChannel() {
	if(session && type == Client)
		delete session;
}

void TestConnectionChannel::start() {
	if(type == Client)
		startClient();
	else
		startServer();
}

void TestConnectionChannel::startClient() {
	if(session) {
		error("Client session already started\n");
		return;
	}

	if(encrypted)
		session = new TestPacketSession<EncryptedSession<PacketSession>>(this);
	else
		session = new TestPacketSession<PacketSession>(this);

	std::string host = getHostname(name);
	session->connect(host.c_str(), 0);
}

void TestConnectionChannel::startServer() {
	if(session) {
		error("Client session already started\n");
		return;
	}

	TestPacketServer* server = new TestPacketServer(this, getHostname(name), encrypted);

	server->start();
}

void TestConnectionChannel::sendPacket(const TS_MESSAGE *packet) {
	if(session)
		session->sendPacket(packet);
}

void TestConnectionChannel::closeSession() {
	if(session)
		session->closeSession();
}

void TestConnectionChannel::onEventReceived(PacketSession *session, Event::Type type) {
	Event event;
	event.type = type;
	event.packet = nullptr;
	event.origineName = name;

	callEventCallback(name, event, session);
}

void TestConnectionChannel::onPacketReceived(PacketSession *session, const TS_MESSAGE *packet) {
	Event event;
	event.type = Event::Packet;
	event.packet = packet;
	event.origineName = name;
	callEventCallback(name, event, session);
}

void TestConnectionChannel::callEventCallback(std::string name, Event event, PacketSession* session) {
	if(eventCallbacks.size() > 0) {
		(eventCallbacks.front())(this, event);
		eventCallbacks.pop_front();
	} else {
		session->closeSession();
	}
}

void TestConnectionChannel::registerSession(PacketSession *session) {
	this->session = session;
}

void TestConnectionChannel::unregisterSession() {
	session = nullptr;
}



std::string TestConnectionChannel::getHostname(std::string serverName) {
	std::string name;
#ifdef WIN32
	name = std::string("pipe:\\\\.\\pipe\\") + serverName;
#else
	name = std::string("pipe:/tmp/") + serverName;
	remove(name.c_str());
#endif

	return name;
}

TS_MESSAGE *TestConnectionChannel::copyMessage(const TS_MESSAGE *packet) {
	TS_MESSAGE * newPacket = (TS_MESSAGE*) malloc(packet->size);
	memcpy(newPacket, packet, packet->size);
	return newPacket;
}
