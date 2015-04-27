#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/PacketEnums.h"
#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_AG_KICK_CLIENT.h"
#include "Packets/TS_AC_SERVER_LIST.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Common.h"
#include "../ClientSession/Common.h"

#include "DesPasswordCipher.h"

namespace AuthServer {

TEST(TS_GA_LOGIN_WITH_LOGOUT, valid_not_ready_hidden) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);


	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLoginEx(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test2", "admin");

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		EXPECT_EQ(0, packet->count);

		channel->closeSession();
		AuthServer::sendGameLogout(&game);
	});

	game.start();
	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_GA_LOGIN_WITH_LOGOUT, valid_account_list_empty_ready) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);


	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLoginEx(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);
		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		sendGameConnectedAccounts(channel, std::vector<AccountInfo>());

		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test2", "admin");

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		EXPECT_EQ(1, packet->count);
		EXPECT_EQ(1, packet->servers[0].server_idx);

		channel->closeSession();
		AuthServer::sendGameLogout(&game);
	});

	game.start();
	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_GA_LOGIN_WITH_LOGOUT, valid_account_list_1) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);


	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLoginEx(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);
		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		std::vector<AccountInfo> accounts;
		AccountInfo a = {"test1", 1, 0, 0, 0, 0, 0};
		accounts.push_back(a);
		sendGameConnectedAccounts(channel, accounts);

		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test2", "admin");
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		EXPECT_EQ(1, packet->count);
		EXPECT_EQ(1, packet->servers[0].server_idx);

		channel->closeSession();
	});
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		channel->start();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "admin");
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_KICK_CLIENT* packet = AGET_PACKET(TS_AG_KICK_CLIENT);

		EXPECT_STREQ("test1", packet->account);
		EXPECT_EQ(TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN, packet->kick_type);

		TS_GA_CLIENT_LOGOUT clientLogout;
		TS_MESSAGE::initMessage(&clientLogout);
		strcpy(clientLogout.account, "test1");
		channel->sendPacket(&clientLogout);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_ALREADY_EXIST, 0);

		channel->closeSession();
		AuthServer::sendGameLogout(&game);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	game.start();
	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_GA_LOGIN_WITH_LOGOUT, valid_account_list_4) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLoginEx(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);
		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		std::vector<AccountInfo> accounts;
		{
			AccountInfo a = {"test1", 1, 0, 0, 0, 0, 0};
			accounts.push_back(a);
		}
		{
			AccountInfo a = {"test2", 2, 0, 0, 0, 0, 0};
			accounts.push_back(a);
		}
		{
			AccountInfo a = {"test3", 3, 0, 0, 0, 0, 0};
			accounts.push_back(a);
		}
		{
			AccountInfo a = {"test4", 4, 0, 0, 0, 0, 0};
			accounts.push_back(a);
		}
		sendGameConnectedAccounts(channel, accounts);

		auth.addYield([](TestConnectionChannel* channel) {
			channel->start();
		}, 100);
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test5", "admin");
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		EXPECT_EQ(1, packet->count);
		EXPECT_EQ(1, packet->servers[0].server_idx);

		channel->closeSession();
	});
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		channel->start();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "admin");
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_KICK_CLIENT* packet = AGET_PACKET(TS_AG_KICK_CLIENT);

		EXPECT_STREQ("test1", packet->account);
		EXPECT_EQ(TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN, packet->kick_type);

		TS_GA_CLIENT_LOGOUT clientLogout;
		TS_MESSAGE::initMessage(&clientLogout);
		strcpy(clientLogout.account, "test1");
		channel->sendPacket(&clientLogout);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_ALREADY_EXIST, 0);

		channel->closeSession();
		AuthServer::sendGameLogout(&game);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	game.start();
	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_GA_LOGIN_WITH_LOGOUT, valid_account_list_1_kick) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	addClientLoginToServerListScenario(auth, AM_Aes, "test2", "admin");
	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		EXPECT_EQ(0, packet->count);

		game.start();
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLoginEx(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);
		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		std::vector<AccountInfo> accounts;
		AccountInfo a = {"test2", 2, 0, 0, 0, 0, 0};
		accounts.push_back(a);
		sendGameConnectedAccounts(channel, accounts);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_KICK_CLIENT* packet = AGET_PACKET(TS_AG_KICK_CLIENT);
		ASSERT_STREQ("test2", packet->account);
		ASSERT_EQ(TS_AG_KICK_CLIENT::KICK_TYPE_DUPLICATED_LOGIN, packet->kick_type);

		auth.closeSession();
		AuthServer::sendGameLogout(channel);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});
	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.start();
	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

} // namespace AuthServer
