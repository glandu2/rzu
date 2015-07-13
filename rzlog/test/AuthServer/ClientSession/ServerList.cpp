#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/TS_AC_SERVER_LIST.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_CA_SERVER_LIST.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/PacketEnums.h"
#include "Common.h"
#include "../GameServerSession/Common.h"

namespace AuthServer {

TEST(TS_CA_SERVER_LIST, empty_list) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	addClientLoginToServerListScenario(auth, AM_Aes, "test2", "admin");

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		EXPECT_EQ(4, packet->last_login_server_idx);
		EXPECT_EQ(0, packet->count);

		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SERVER_LIST, not_authenticated) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		TS_CA_SERVER_LIST serverListPacket;
		TS_MESSAGE::initMessage(&serverListPacket);
		channel->sendPacket(&serverListPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SERVER_LIST, not_authenticated_epic2) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		TS_CA_VERSION versionPacket;
		TS_MESSAGE::initMessage(&versionPacket);
		strcpy(versionPacket.szVersion, "200609280");
		channel->sendPacket(&versionPacket);

		TS_CA_SERVER_LIST serverListPacket;
		TS_MESSAGE::initMessage(&serverListPacket);
		channel->sendPacket(&serverListPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SERVER_LIST, two_gs_list) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel game1(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel game2(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game1.start();

	addGameLoginScenario(game1, 10, "Server 10", "http://www.example.com/index10.html", true, "127.0.0.10", 4710,
						 [&game2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		game2.start();
	});

	addGameLoginScenario(game2, 1, "Server 1", "http://www.example.com/index1.html", false, "127.0.0.1", 4610,
						 [&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test3", "admin");

	auth.addCallback([&game1, &game2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		ASSERT_EQ(2, packet->count);
		const TS_AC_SERVER_LIST::TS_SERVER_INFO* serverInfo;

		EXPECT_TRUE(packet->servers[0].server_idx == 10 || packet->servers[0].server_idx == 1);
		EXPECT_TRUE(packet->servers[1].server_idx == 10 || packet->servers[1].server_idx == 1);

		if(packet->servers[0].server_idx == 10)
			serverInfo = &packet->servers[0];
		else
			serverInfo = &packet->servers[1];

		EXPECT_EQ(10, serverInfo->server_idx);
		EXPECT_STREQ("Server 10", serverInfo->server_name);
		EXPECT_TRUE(serverInfo->is_adult_server);
		EXPECT_STREQ("http://www.example.com/index10.html", serverInfo->server_screenshot_url);
		EXPECT_STREQ("127.0.0.10", serverInfo->server_ip);
		EXPECT_EQ(4710, serverInfo->server_port);
		EXPECT_EQ(0, serverInfo->user_ratio);

		if(packet->servers[0].server_idx == 1)
			serverInfo = &packet->servers[0];
		else
			serverInfo = &packet->servers[1];

		EXPECT_EQ(1, serverInfo->server_idx);
		EXPECT_STREQ("Server 1", serverInfo->server_name);
		EXPECT_FALSE(serverInfo->is_adult_server);
		EXPECT_STREQ("http://www.example.com/index1.html", serverInfo->server_screenshot_url);
		EXPECT_STREQ("127.0.0.1", serverInfo->server_ip);
		EXPECT_EQ(4610, serverInfo->server_port);
		EXPECT_EQ(0, serverInfo->user_ratio);

		channel->closeSession();
		game1.closeSession();
		game2.closeSession();
	});

	test.addChannel(&game1);
	test.addChannel(&game2);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SERVER_LIST, two_epic2_gs_list) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel game1(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel game2(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game1.start();

	addGameLoginScenario(game1, 10, "Server 10", "http://www.example.com/index10.html", true, "127.0.0.10", 4710,
						 [&game2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		game2.start();
	});

	addGameLoginScenario(game2, 1, "Server 1", "http://www.example.com/index1.html", false, "127.0.0.1", 4610,
						 [&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Des, "test1", "admin", nullptr, "200609280");

	auth.addCallback([&game1, &game2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST_EPIC2* packet = AGET_PACKET(TS_AC_SERVER_LIST_EPIC2);

		ASSERT_EQ(2, packet->count);
		const TS_AC_SERVER_LIST_EPIC2::TS_SERVER_INFO* serverInfo;

		EXPECT_TRUE(packet->servers[0].server_idx == 10 || packet->servers[0].server_idx == 1);
		EXPECT_TRUE(packet->servers[1].server_idx == 10 || packet->servers[1].server_idx == 1);

		if(packet->servers[0].server_idx == 10)
			serverInfo = &packet->servers[0];
		else
			serverInfo = &packet->servers[1];

		EXPECT_EQ(10, serverInfo->server_idx);
		EXPECT_STREQ("Server 10", serverInfo->server_name);
		EXPECT_STREQ("127.0.0.10", serverInfo->server_ip);
		EXPECT_EQ(4710, serverInfo->server_port);
		EXPECT_EQ(0, serverInfo->user_ratio);

		if(packet->servers[0].server_idx == 1)
			serverInfo = &packet->servers[0];
		else
			serverInfo = &packet->servers[1];

		EXPECT_EQ(1, serverInfo->server_idx);
		EXPECT_STREQ("Server 1", serverInfo->server_name);
		EXPECT_STREQ("127.0.0.1", serverInfo->server_ip);
		EXPECT_EQ(4610, serverInfo->server_port);
		EXPECT_EQ(0, serverInfo->user_ratio);


		channel->closeSession();
		game1.closeSession();
		game2.closeSession();
	});

	test.addChannel(&game1);
	test.addChannel(&game2);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SERVER_LIST, non_null_terminated_gs_info) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.start();

	addGameLoginScenario(game,
						 2,
						 "21_chars_aaaaaaaaaaaa",
						 "http://www.example.com/index_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.html",
						 true,
						 "121.131.165.156.",
						 65312,
						 [&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test4", "admin");

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		ASSERT_EQ(1, packet->count);
		const TS_AC_SERVER_LIST::TS_SERVER_INFO* serverInfo = &packet->servers[0];

		EXPECT_EQ(2, serverInfo->server_idx);
		EXPECT_STREQ("21_chars_aaaaaaaaaaa", serverInfo->server_name);
		EXPECT_TRUE(serverInfo->is_adult_server);
		EXPECT_STREQ("http://www.example.com/index_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.htm", serverInfo->server_screenshot_url);
		EXPECT_STREQ("121.131.165.156", serverInfo->server_ip);
		EXPECT_EQ(65312, serverInfo->server_port);

		channel->closeSession();
		game.closeSession();
	});

	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

} // namespace AuthServer
