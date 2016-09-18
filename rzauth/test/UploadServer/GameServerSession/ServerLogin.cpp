#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "PacketEnums.h"
#include "UploadGame/TS_SU_LOGIN.h"
#include "UploadGame/TS_US_LOGIN_RESULT.h"

namespace UploadServer {

TEST(TS_SU_LOGIN, valid) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_SU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		strcpy(loginPacket.server_name, "game002");
		channel->sendPacket(&loginPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_LOGIN_RESULT* packet = AGET_PACKET(TS_US_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_SU_LOGIN, invalid) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_SU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		strcpy(loginPacket.server_name, "game/001");
		channel->sendPacket(&loginPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_LOGIN_RESULT* packet = AGET_PACKET(TS_US_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_INVALID_TEXT, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_SU_LOGIN, large) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_SU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		memset(loginPacket.server_name, 'g', sizeof(loginPacket.server_name));
		channel->sendPacket(&loginPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_LOGIN_RESULT* packet = AGET_PACKET(TS_US_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

} // namespace UploadServer
