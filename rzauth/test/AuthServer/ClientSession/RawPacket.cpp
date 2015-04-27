#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/PacketBaseMessage.h"

namespace AuthServer {

TEST(RAW_PACKET, large_size) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		static const int PACKET_SIZE = 1*1024*1024; // 1MB

		TS_MESSAGE *packet = (TS_MESSAGE*)calloc(PACKET_SIZE, 1);
		packet->size = PACKET_SIZE;
		packet->id = 0;
		packet->msg_check_sum = 0;

		channel->sendPacket(packet);
		free(packet);
		channel->closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST(RAW_PACKET, unknown_id) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		static const int PACKET_SIZE = 1*1024; // 1kB

		TS_MESSAGE* packet = (TS_MESSAGE*)calloc(PACKET_SIZE, 1);
		packet->size = PACKET_SIZE;
		packet->id = 32165;
		packet->msg_check_sum = 0;

		channel->sendPacket(packet);
		free(packet);
		channel->closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

} // namespace AuthServer
