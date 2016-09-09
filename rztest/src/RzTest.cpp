#include "RzTest.h"
#include "Packet/PacketBaseMessage.h"
#include "Core/EventLoop.h"
#include "TestPacketSession.h"
#include "TestPacketServer.h"
#include "NetSession/EncryptedSession.h"
#include <tuple>

RzTest::RzTest()
{
}

void RzTest::addChannel(TestConnectionChannel* channel) {
	channels.push_back(channel);
	channel->setTest(this);
}

void RzTest::abortTest() {
	for(auto it = channels.begin(); it != channels.end(); ++it) {
		TestConnectionChannel* channel = *it;
		channel->closeSession();
	}
}

void RzTest::run() {
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
	for(auto it = channels.begin(); it != channels.end(); ++it) {
		TestConnectionChannel* channel = *it;
		EXPECT_EQ(0, channel->getPendingCallbacks()) << channel->getPort();
	}
}
