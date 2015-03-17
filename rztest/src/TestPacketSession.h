#ifndef TESTPACKETSESSION_H
#define TESTPACKETSESSION_H

#include "PacketSession.h"
#include "TestConnectionChannel.h"
#include "Extern.h"

class TestPacketServer;

template<class T>
class RZTEST_EXTERN TestPacketSession : public T
{
public:
	TestPacketSession(TestConnectionChannel* channel);

	void onConnected();

	void onDisconnected(bool causedByRemote);

	void onPacketReceived(const TS_MESSAGE *packet);

private:
	TestConnectionChannel* channel;
};

template<class T>
TestPacketSession<T>::TestPacketSession(TestConnectionChannel *channel)
	: channel(channel) {}

template<class T>
void TestPacketSession<T>::onConnected() {
	channel->onEventReceived(this, TestConnectionChannel::Event::Connection);
}

template<class T>
void TestPacketSession<T>::onDisconnected(bool causedByRemote) {
	channel->onEventReceived(this, TestConnectionChannel::Event::Disconnection);
}

template<class T>
void TestPacketSession<T>::onPacketReceived(const TS_MESSAGE *packet) {
	channel->onPacketReceived(this, packet);
}

#endif // TESTPACKETSESSION_H
