#include "TestConnectionChannel.h"
#include "Config/ConfigParamVal.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include "RzTest.h"
#include "TestPacketServer.h"
#include "TestPacketSession.h"

TestConnectionChannel::TestConnectionChannel(TestConnectionChannel::Type type,
                                             cval<std::string>& host,
                                             cval<int>& port,
                                             bool encrypted)
    : type(type), host(host), port(port), session(nullptr), server(nullptr), test(nullptr) {
	switch(type) {
		case Client:
			if(encrypted)
				session = new TestPacketSession<EncryptedSession<PacketSession>>(this);
			else
				session = new TestPacketSession<PacketSession>(this);
			break;
		case Server:
			server = new TestPacketServer(this, host, port, encrypted);
			break;
	}
}

TestConnectionChannel::~TestConnectionChannel() {
	if(session && type == Client)
		delete session;
	if(server)
		delete server;
}

void TestConnectionChannel::YieldData::executeTimerForYield() {
	callback(instance);
	delete this;
}

void TestConnectionChannel::addYield(YieldCallback callback, int time) {
	YieldData* yieldData = new YieldData;

	yieldData->callback = callback;
	yieldData->instance = this;

	yieldData->timer.start(yieldData, &YieldData::executeTimerForYield, time, 0);
}

int TestConnectionChannel::getPort() {
	return port.get();
}

void TestConnectionChannel::start() {
	if(type == Client)
		startClient();
	else
		startServer();
}

void TestConnectionChannel::startClient() {
	if(session && session->getState() != Stream::UnconnectedState) {
		log(LL_Error, "Client session to %s:%d already started\n", host.get().c_str(), port.get());
		return;
	} else if(!session) {
		log(LL_Error, "Attempt to start client for %s:%d but there is no session\n", host.get().c_str(), port.get());
		return;
	}

	session->connect(host.get().c_str(), port);
}

void TestConnectionChannel::startServer() {
	if(session && session->getState() != Stream::UnconnectedState) {
		log(LL_Error, "Server session on %s:%d already started\n", host.get().c_str(), port.get());
		return;
	} else if(!server) {
		log(LL_Error,
		    "Attempt to start server for %s:%d but there is no server instance\n",
		    host.get().c_str(),
		    port.get());
		return;
	} else if(server->isStarted()) {
		log(LL_Error, "Attempt to start server for %s:%d but already started\n", host.get().c_str(), port.get());
		return;
	}

	server->start();
}

void TestConnectionChannel::sendPacket(const TS_MESSAGE* packet) {
	log(LL_Debug, "[%s:%d] Sending packet id %d\n", host.get().c_str(), port.get(), packet->id);
	if(session)
		session->sendPacket(packet);
}

void TestConnectionChannel::closeSession() {
	log(LL_Debug, "[%s:%d] Closing session\n", host.get().c_str(), port.get());
	if(session)
		session->closeSession();
	if(server)
		server->stop();
}

void TestConnectionChannel::abortTest() {
	eventCallbacks.clear();
	closeSession();
}

void TestConnectionChannel::onEventReceived(PacketSession* session, Event::Type type) {
	Event event;
	event.type = type;
	event.packet = nullptr;
	callEventCallback(event, session);
}

void TestConnectionChannel::onPacketReceived(PacketSession* session, const TS_MESSAGE* packet) {
	Event event;
	event.type = Event::Packet;
	event.packet = packet;
	callEventCallback(event, session);
}

void TestConnectionChannel::callEventCallback(Event event, PacketSession* session) {
	if(eventCallbacks.size() > 0) {
		switch(event.type) {
			case Event::Packet:
				log(LL_Debug,
				    "[%s:%d] Received event type Packet, packet id %d\n",
				    host.get().c_str(),
				    port.get(),
				    event.packet ? event.packet->id : 0);
				break;
			case Event::Connection:
				log(LL_Debug, "[%s:%d] Received event type connection\n", host.get().c_str(), port.get());
				break;
			case Event::Disconnection:
				log(LL_Debug, "[%s:%d] Received event type disconnection\n", host.get().c_str(), port.get());
				break;
		}
		(eventCallbacks.front())(this, event);
		eventCallbacks.pop_front();
	} else if(eventCallbacks.size() == 0) {
		log(LL_Debug, "No more callbacks, closing connection\n");
		closeSession();
	}
	if(::testing::Test::HasFatalFailure()) {
		log(LL_Debug, "Got fatal error, aborting test\n");
		if(test)
			test->abortTest();
	}
}

void TestConnectionChannel::registerSession(PacketSession* session) {
	if(this->session != nullptr) {
		log(LL_Error, "Multiple register\n");
	}
	this->session = session;
}

void TestConnectionChannel::unregisterSession(PacketSession* session) {
	if(this->session == session)
		this->session = nullptr;
}

bool TestConnectionChannel::isConnected() {
	return session && session->getState() != Stream::UnconnectedState;
}

TS_MESSAGE* TestConnectionChannel::copyMessage(const TS_MESSAGE* packet) {
	TS_MESSAGE* newPacket = (TS_MESSAGE*) malloc(packet->size);
	memcpy(newPacket, packet, packet->size);
	return newPacket;
}
