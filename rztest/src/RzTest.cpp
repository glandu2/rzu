#include "RzTest.h"
#include "Core/EventLoop.h"
#include "NetSession/EncryptedSession.h"
#include "Packet/PacketBaseMessage.h"
#include "TestPacketServer.h"
#include "TestPacketSession.h"
#include <tuple>

RzTest::RzTest() {
	timeoutTimer.unref();
}

void RzTest::addChannel(TestConnectionChannel* channel) {
	channels.push_back(channel);
	channel->setTest(this);
}

void RzTest::abortTest() {
	for(auto it = channels.begin(); it != channels.end(); ++it) {
		TestConnectionChannel* channel = *it;
		channel->abortTest();
	}
}

void RzTest::run(int timeoutMs) {
	if(timeoutMs)
		timeoutTimer.start(this, &RzTest::onTestTimeout, timeoutMs, 0);

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
	for(auto it = channels.begin(); it != channels.end(); ++it) {
		TestConnectionChannel* channel = *it;
		EXPECT_EQ(0, channel->getPendingCallbacks()) << channel->getPort();
	}
	if(timeoutMs)
		timeoutTimer.stop();
}

void RzTest::updateObjectName() {
	const ::testing::TestInfo* test_info = ::testing::UnitTest::GetInstance()->current_test_info();

	if(test_info) {
		char buffer[128];
		snprintf(buffer, sizeof(buffer), "RzTest:%s.%s", test_info->test_case_name(), test_info->name());

		setObjectName(buffer);
	} else {
		Object::updateObjectName();
	}
}

void RzTest::onTestTimeout() {
	EXPECT_TRUE(false) << "Test timeout, closing all connections";
	abortTest();
}
