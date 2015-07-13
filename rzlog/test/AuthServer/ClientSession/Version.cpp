#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_SC_RESULT.h"

namespace AuthServer {

TEST(TS_CA_VERSION, playercount) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_CA_VERSION version;
		TS_MESSAGE::initMessage(&version);
		strcpy(version.szVersion, "TEST");
		channel->sendPacket(&version);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_SC_RESULT* packet = AGET_PACKET(TS_SC_RESULT);

		int playerCount = packet->value ^ 0xADADADAD;

		EXPECT_EQ(0, playerCount);
		EXPECT_EQ(0, packet->result);

		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST(TS_CA_VERSION, version) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_CA_VERSION version;
		TS_MESSAGE::initMessage(&version);
		strcpy(version.szVersion, "INFO");
		channel->sendPacket(&version);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_SC_RESULT* packet = AGET_PACKET(TS_SC_RESULT);
		uint32_t versionHex = ((uint32_t)packet->value) ^ 0xADADADAD;

		EXPECT_NE(0, versionHex);
		EXPECT_NE(INT32_MIN, versionHex);
		EXPECT_NE(INT32_MAX, versionHex);
		EXPECT_NE(UINT32_MAX, versionHex);
		EXPECT_EQ(0, packet->result);

		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST(TS_CA_VERSION, nonnullterminated) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_CA_VERSION version;
		TS_MESSAGE::initMessage(&version);
		memset(version.szVersion, 1, sizeof(version.szVersion));
		channel->sendPacket(&version);
		strcpy(version.szVersion, "INFO");
		channel->sendPacket(&version);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_SC_RESULT* packet = AGET_PACKET(TS_SC_RESULT);
		uint32_t versionHex = ((uint32_t)packet->value) ^ 0xADADADAD;

		EXPECT_NE(0, versionHex);
		EXPECT_NE(INT32_MIN, versionHex);
		EXPECT_NE(INT32_MAX, versionHex);
		EXPECT_NE(UINT32_MAX, versionHex);
		EXPECT_EQ(0, packet->result);

		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

} // namespace AuthServer
