#include "TestConnectionChannel.h"
#include "TestPacketServer.h"
#include "TestPacketSession.h"
#include "NetSession/EncryptedSession.h"
#include "Config/ConfigParamVal.h"
#include "RzTest.h"
#include "Core/EventLoop.h"

TestConnectionChannel::~TestConnectionChannel() {
	if(session && type == Client)
		delete session;
}

void TestConnectionChannel::executeTimerForYield(uv_timer_t* timer) {
	YieldData* instance = (YieldData*) timer->data;

	instance->callback(instance->instance);

	uv_close((uv_handle_t*)timer, &closeTimerForYield);
}

void TestConnectionChannel::closeTimerForYield(uv_handle_t* timer) {
	YieldData* instance = (YieldData*) timer->data;
	delete instance;
}

void TestConnectionChannel::addYield(YieldCallback callback, int time) {
	YieldData *yieldData = new YieldData;

	uv_timer_init(EventLoop::getLoop(), &yieldData->timer);

	yieldData->callback = callback;
	yieldData->timer.data = yieldData;
	yieldData->instance = this;

	uv_timer_start(&yieldData->timer, &executeTimerForYield, time, 0);
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
		error("Client session to %s:%d already started\n", host.get().c_str(), port.get());
		return;
	}

	if(encrypted)
		session = new TestPacketSession<EncryptedSession<PacketSession>>(this);
	else
		session = new TestPacketSession<PacketSession>(this);

	session->connect(host.get().c_str(), port);
}

void TestConnectionChannel::startServer() {
	if(session && session->getState() != Stream::UnconnectedState) {
		error("Server session on %s:%d already started\n", host.get().c_str(), port.get());
		return;
	}

	server = new TestPacketServer(this, host, port, encrypted);

	server->start();
}

void TestConnectionChannel::sendPacket(const TS_MESSAGE *packet) {
	debug("[%s:%d] Sending packet id %d\n", host.get().c_str(), port.get(), packet->id);
	if(session)
		session->sendPacket(packet);
}

void TestConnectionChannel::closeSession() {
	debug("[%s:%d] Closing session\n", host.get().c_str(), port.get());
	if(session)
		session->closeSession();
	if(server)
		server->stop();
}

void TestConnectionChannel::onEventReceived(PacketSession *session, Event::Type type) {
	Event event;
	event.type = type;
	event.packet = nullptr;
	callEventCallback(event, session);
}

void TestConnectionChannel::onPacketReceived(PacketSession *session, const TS_MESSAGE *packet) {
	Event event;
	event.type = Event::Packet;
	event.packet = packet;
	callEventCallback(event, session);
}

void TestConnectionChannel::callEventCallback(Event event, PacketSession* session) {
	if(eventCallbacks.size() > 0) {
		switch(event.type) {
			case Event::Packet:
				debug("[%s:%d] Received event type Packet, packet id %d\n", host.get().c_str(), port.get(), event.packet->id);
				break;
			case Event::Connection:
				debug("[%s:%d] Received event type connection\n", host.get().c_str(), port.get());
				break;
			case Event::Disconnection:
				debug("[%s:%d] Received event type disconnection\n", host.get().c_str(), port.get());
				break;
		}
		(eventCallbacks.front())(this, event);
		eventCallbacks.pop_front();
	} else if(eventCallbacks.size() == 0) {
		debug("No more callbacks, closing connection\n");
		closeSession();
	}
	if(::testing::Test::HasFatalFailure()) {
		debug("Got fatal error, aborting test\n");
		eventCallbacks.clear();
		closeSession();
		if(test)
			test->abortTest();
	}
}

void TestConnectionChannel::registerSession(PacketSession *session) {
	if(this->session != nullptr) {
		error("Multiple register\n");
	}
	this->session = session;
}

void TestConnectionChannel::unregisterSession(PacketSession *session) {
	if(this->session == session)
		this->session = nullptr;
}

TS_MESSAGE *TestConnectionChannel::copyMessage(const TS_MESSAGE *packet) {
	TS_MESSAGE * newPacket = (TS_MESSAGE*) malloc(packet->size);
	memcpy(newPacket, packet, packet->size);
	return newPacket;
}
