#include "../GlobalConfig.h"
#include "PacketEnums.h"
#include "RzTest.h"
#include "UploadGame/TS_SU_LOGIN.h"
#include "UploadGame/TS_SU_REQUEST_UPLOAD.h"
#include "UploadGame/TS_US_LOGIN_RESULT.h"
#include "UploadGame/TS_US_REQUEST_UPLOAD.h"
#include "UploadGame/TS_US_UPLOAD.h"
#include "gtest/gtest.h"

namespace UploadServer {

TEST(TS_SU_REQUEST_UPLOAD, valid) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_SU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		strcpy(loginPacket.server_name, "game003");
		channel->sendPacket(&loginPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_LOGIN_RESULT* packet = AGET_PACKET(TS_US_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		TS_SU_REQUEST_UPLOAD uploadPacket;
		TS_MESSAGE::initMessage(&uploadPacket);

		uploadPacket.client_id = 0;
		uploadPacket.account_id = 1;
		uploadPacket.guild_sid = 2;
		uploadPacket.one_time_password = 3;
		uploadPacket.type = 4;

		channel->sendPacket(&uploadPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_REQUEST_UPLOAD* packet = AGET_PACKET(TS_US_REQUEST_UPLOAD);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_SU_REQUEST_UPLOAD, double_request) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_SU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		strcpy(loginPacket.server_name, "game004");
		channel->sendPacket(&loginPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_LOGIN_RESULT* packet = AGET_PACKET(TS_US_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		TS_SU_REQUEST_UPLOAD uploadPacket;
		TS_MESSAGE::initMessage(&uploadPacket);

		uploadPacket.client_id = 0;
		uploadPacket.account_id = 1;
		uploadPacket.guild_sid = 2;
		uploadPacket.one_time_password = 3;
		uploadPacket.type = 4;

		channel->sendPacket(&uploadPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_REQUEST_UPLOAD* packet = AGET_PACKET(TS_US_REQUEST_UPLOAD);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		TS_SU_REQUEST_UPLOAD uploadPacket;
		TS_MESSAGE::initMessage(&uploadPacket);

		uploadPacket.client_id = 0;
		uploadPacket.account_id = 10;
		uploadPacket.guild_sid = 20;
		uploadPacket.one_time_password = 30;
		uploadPacket.type = 40;

		channel->sendPacket(&uploadPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_REQUEST_UPLOAD* packet = AGET_PACKET(TS_US_REQUEST_UPLOAD);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

}  // namespace UploadServer
