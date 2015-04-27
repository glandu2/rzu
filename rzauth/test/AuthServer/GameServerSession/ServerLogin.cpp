#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/PacketEnums.h"
#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Common.h"

#include "DesPasswordCipher.h"

namespace AuthServer {

TEST(TS_GA_LOGIN, valid) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_LOGIN, double_login) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	static_assert(sizeof(((TS_GA_LOGIN*)0)->server_name) == 21, "Test expect a name field size of 21");

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 2, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		AuthServer::sendGameLogin(channel, 3, "Server name 1", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_INVALID_ARGUMENT, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_LOGIN, duplicate_index) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel game2(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 4, "Server name 2", "http://www.example.com/index2.html", true, "121.131.165.157", 4517);
	});

	game.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		game2.start();
	});

	game2.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 4, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game2.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_ALREADY_EXIST, packet->result);

		channel->closeSession();
		game.closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.addChannel(&game2);
	test.run();
}

TEST(TS_GA_LOGIN, long_values) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	static_assert(sizeof(((TS_GA_LOGIN*)0)->server_name) == 21, "Test expect a name field size of 21");

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel,
							  5,
							  "21_chars_aaaaaaaaaaaa",
							  "http://www.example.com/index_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.html",
							  true,
							  "121.131.165.156.",
							  65312);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

} // namespace AuthServer
