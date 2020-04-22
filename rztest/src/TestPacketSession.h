#pragma once

//#include "NetSession/PacketSession.h"
#include "Core/EventChain.h"
#include "Extern.h"
#include "Packet/PacketStructsName.h"
#include "TestConnectionChannel.h"

class TestPacketServer;
class PacketSession;
class SocketSession;

template<class T> class RZTEST_EXTERN TestPacketSession : public T {
public:
	TestPacketSession(TestConnectionChannel* channel);
	~TestPacketSession();

	EventChain<SocketSession> onConnected();
	EventChain<SocketSession> onDisconnected(bool causedByRemote);
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

private:
	TestConnectionChannel* channel;
};

template<class T>
TestPacketSession<T>::TestPacketSession(TestConnectionChannel* channel)
    : T(SessionType::Any, SessionPacketOrigin::Any, EPIC_LATEST), channel(channel) {
	channel->registerSession(this);
}

template<class T> TestPacketSession<T>::~TestPacketSession() {
	channel->unregisterSession(this);
}

template<class T> EventChain<SocketSession> TestPacketSession<T>::onConnected() {
	channel->onEventReceived(this, TestConnectionChannel::Event::Connection);
	return T::onConnected();
}

template<class T> EventChain<SocketSession> TestPacketSession<T>::onDisconnected(bool causedByRemote) {
	channel->onEventReceived(this, TestConnectionChannel::Event::Disconnection);
	return T::onDisconnected(causedByRemote);
}

template<class T> EventChain<PacketSession> TestPacketSession<T>::onPacketReceived(const TS_MESSAGE* packet) {
	channel->onPacketReceived(this, packet);
	return T::onPacketReceived(packet);
}

