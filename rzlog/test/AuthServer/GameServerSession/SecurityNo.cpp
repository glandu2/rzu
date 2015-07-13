#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_SECURITY_NO_CHECK.h"
#include "Packets/TS_GA_SECURITY_NO_CHECK.h"
#include "Common.h"

#include "DesPasswordCipher.h"

namespace AuthServer {

TEST(TS_GA_SECURITY_NO_CHECK, valid) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 20, "Server name 20", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		strcpy(securityNoPacket.account, "test1");
		strcpy(securityNoPacket.security, "admin");
		securityNoPacket.mode = 42;

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK);

		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK), packet->size);
		EXPECT_EQ(1, packet->result);
		EXPECT_STREQ("test1", packet->account);
		EXPECT_EQ(42, packet->mode);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_SECURITY_NO_CHECK, wrong) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 21, "Server name 21", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		strcpy(securityNoPacket.account, "test1");
		strcpy(securityNoPacket.security, "adzazd");
		securityNoPacket.mode = 0xFFFFFFFF;

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK);

		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK), packet->size);
		EXPECT_EQ(0, packet->result);
		EXPECT_STREQ("test1", packet->account);
		EXPECT_EQ(0xFFFFFFFF, packet->mode);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_SECURITY_NO_CHECK, long_account) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 22, "Server name 22", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		strcpy(securityNoPacket.account, "60_chars_long_account_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
		strcpy(securityNoPacket.security, "admin");
		securityNoPacket.mode = 0xFFFFFFFF;

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK);

		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK), packet->size);
		EXPECT_EQ(0, packet->result);
		EXPECT_STREQ("60_chars_long_account_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", packet->account);
		EXPECT_EQ(0xFFFFFFFF, packet->mode);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_SECURITY_NO_CHECK, long_securityno) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 23, "Server name 23", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		strcpy(securityNoPacket.account, "test1");
		strcpy(securityNoPacket.security, "19_chars_long_secur");
		securityNoPacket.mode = 0xFFF5FFFF;

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK);

		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK), packet->size);
		EXPECT_EQ(0, packet->result);
		EXPECT_STREQ("test1", packet->account);
		EXPECT_EQ(0xFFF5FFFF, packet->mode);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_SECURITY_NO_CHECK, garbage_data) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 24, "Server name 24", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		memset(securityNoPacket.account, 'a', sizeof(securityNoPacket.account));
		memset(securityNoPacket.security, 127, sizeof(securityNoPacket.security));
		securityNoPacket.mode = 0xFFF5FFFF;

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK);

		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK), packet->size);
		EXPECT_EQ(0, packet->result);
		EXPECT_STREQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", packet->account);
		EXPECT_EQ(0xFFF5FFFF, packet->mode);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_SECURITY_NO_CHECK, double_query) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	uint32_t modeDone = 0;

	addGameLoginScenario(game, 20, "Server name 20", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		strcpy(securityNoPacket.account, "test1");
		strcpy(securityNoPacket.security, "admin");
		securityNoPacket.mode = 42;

		channel->sendPacket(&securityNoPacket);

		strcpy(securityNoPacket.account, "test2");
		strcpy(securityNoPacket.security, "admin");
		securityNoPacket.mode = 54;

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([&modeDone](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK);
		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK), packet->size);
		EXPECT_TRUE(packet->mode == 42 || packet->mode == 54);

		if(packet->mode == 42) {
			EXPECT_EQ(1, packet->result);
			EXPECT_STREQ("test1", packet->account);
			EXPECT_EQ(42, packet->mode);

		} else {
			EXPECT_EQ(0, packet->result);
			EXPECT_STREQ("test2", packet->account);
			EXPECT_EQ(54, packet->mode);
		}

		modeDone = packet->mode;
	});

	game.addCallback([&modeDone](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK);
		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK), packet->size);
		EXPECT_TRUE(packet->mode == 42 || packet->mode == 54);
		EXPECT_NE(modeDone, packet->mode);

		if(packet->mode == 42) {
			EXPECT_EQ(1, packet->result);
			EXPECT_STREQ("test1", packet->account);
			EXPECT_EQ(42, packet->mode);
		} else {
			EXPECT_EQ(0, packet->result);
			EXPECT_STREQ("test2", packet->account);
			EXPECT_EQ(54, packet->mode);
		}

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_SECURITY_NO_CHECK, query_and_close) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 20, "Server name 20", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		strcpy(securityNoPacket.account, "test1");
		strcpy(securityNoPacket.security, "admin");
		securityNoPacket.mode = 42;

		channel->sendPacket(&securityNoPacket);
		channel->closeSession();
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

// Epic 5 tests
TEST(TS_GA_SECURITY_NO_CHECK, valid_epic5) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 24, "Server name 24", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK_EPIC5 securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		strcpy(securityNoPacket.account, "test1");
		strcpy(securityNoPacket.security, "admin");

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK_EPIC5* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK_EPIC5);

		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK_EPIC5), packet->size);
		EXPECT_EQ(1, packet->result);
		EXPECT_STREQ("test1", packet->account);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_SECURITY_NO_CHECK, garbage_data_epic5) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 24, "Server name 24", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_GA_SECURITY_NO_CHECK_EPIC5 securityNoPacket;
		TS_MESSAGE::initMessage(&securityNoPacket);

		memset(securityNoPacket.account, 'a', sizeof(securityNoPacket.account));
		memset(securityNoPacket.security, 127, sizeof(securityNoPacket.security));

		channel->sendPacket(&securityNoPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_SECURITY_NO_CHECK_EPIC5* packet = AGET_PACKET(TS_AG_SECURITY_NO_CHECK_EPIC5);

		EXPECT_EQ(sizeof(TS_AG_SECURITY_NO_CHECK_EPIC5), packet->size);
		EXPECT_EQ(0, packet->result);
		EXPECT_STREQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", packet->account);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

} // namespace AuthServer
