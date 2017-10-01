#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "AuthGame/TS_AG_LOGIN_RESULT.h"
#include "AuthGame/TS_GA_ACCOUNT_LIST.h"
#include "AuthGame/TS_GA_CLIENT_KICK_FAILED.h"
#include "AuthGame/TS_GA_CLIENT_LOGIN.h"
#include "AuthGame/TS_GA_CLIENT_LOGOUT.h"
#include "AuthGame/TS_GA_LOGIN.h"
#include "AuthGame/TS_GA_LOGOUT.h"
#include "Common.h"
#include "GlobalConfig.h"
#include "PacketEnums.h"
#include "RzTest.h"
#include "gtest/gtest.h"

#include "Cipher/DesPasswordCipher.h"

namespace AuthServer {

// GS connect, connect to auth, account list, client connection, client disconnect
TEST(TS_GA_CLIENT_LOGIN, client_connect_disconnect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(
		    channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		EXPECT_EQ(1, packet->server_idx);
		EXPECT_STREQ("Server name", packet->server_name);
		EXPECT_STREQ("http://www.example.com/index.html", packet->server_screenshot_url);
		EXPECT_FALSE(packet->is_adult_server);
		EXPECT_STREQ("121.131.165.156", packet->server_ip);
		EXPECT_EQ(4516, packet->server_port);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		TS_GA_CLIENT_LOGIN clientLoginPacket;
		TS_MESSAGE::initMessage(&clientLoginPacket);
		strcpy(clientLoginPacket.account, "account");
		clientLoginPacket.one_time_key = 42;
		game.sendPacket(&clientLoginPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_CLIENT_LOGIN* packet = AGET_PACKET(TS_GA_CLIENT_LOGIN);
		EXPECT_STREQ("account", packet->account);
		EXPECT_EQ(42, packet->one_time_key);

		TS_AG_CLIENT_LOGIN_EXTENDED clientLoginPacket;
		TS_MESSAGE::initMessage(&clientLoginPacket);
		strcpy(clientLoginPacket.account, "account");
		clientLoginPacket.nAccountID = 5;
		clientLoginPacket.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&clientLoginPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_EQ(5, packet->nAccountID);
		EXPECT_STREQ("account", packet->account);

		TS_GA_CLIENT_LOGOUT clientLogoutPacket;
		TS_MESSAGE::initMessage(&clientLogoutPacket);
		strcpy(clientLogoutPacket.account, "account");
		clientLogoutPacket.nContinuousPlayTime = 4;
		channel->sendPacket(&clientLogoutPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_CLIENT_LOGOUT* packet = AGET_PACKET(TS_GA_CLIENT_LOGOUT);
		EXPECT_STREQ("account", packet->account);
		EXPECT_EQ(4, packet->nContinuousPlayTime);

		channel->closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		channel->start();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		game.closeSession();
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGOUT* packet = AGET_PACKET(TS_GA_LOGOUT);
		channel->closeSession();
	});

	auth.start();
	game.start();
	test.addChannel(&auth);
	test.addChannel(&game);
	test.run();
}

// GS connect, connect to auth, account list, client connection, auth disconnect, client disconnect, auth connect
TEST(TS_GA_CLIENT_LOGIN, client_connect_auth_disconnect_client_disconnect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(
		    channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		EXPECT_EQ(1, packet->server_idx);
		EXPECT_STREQ("Server name", packet->server_name);
		EXPECT_STREQ("http://www.example.com/index.html", packet->server_screenshot_url);
		EXPECT_FALSE(packet->is_adult_server);
		EXPECT_STREQ("121.131.165.156", packet->server_ip);
		EXPECT_EQ(4516, packet->server_port);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		TS_GA_CLIENT_LOGIN clientLoginPacket;
		TS_MESSAGE::initMessage(&clientLoginPacket);
		strcpy(clientLoginPacket.account, "account");
		clientLoginPacket.one_time_key = 42;
		game.sendPacket(&clientLoginPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_CLIENT_LOGIN* packet = AGET_PACKET(TS_GA_CLIENT_LOGIN);
		EXPECT_STREQ("account", packet->account);
		EXPECT_EQ(42, packet->one_time_key);

		TS_AG_CLIENT_LOGIN_EXTENDED clientLoginPacket;
		TS_MESSAGE::initMessage(&clientLoginPacket);
		strcpy(clientLoginPacket.account, "account");
		clientLoginPacket.nAccountID = 5;
		clientLoginPacket.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&clientLoginPacket);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_EQ(5, packet->nAccountID);
		EXPECT_STREQ("account", packet->account);

		auth.closeSession();
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		TS_GA_CLIENT_LOGOUT clientLogoutPacket;
		TS_MESSAGE::initMessage(&clientLogoutPacket);
		strcpy(clientLogoutPacket.account, "account");
		clientLogoutPacket.nContinuousPlayTime = 4;
		game.sendPacket(&clientLogoutPacket);

		channel->start();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		game.closeSession();
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGOUT* packet = AGET_PACKET(TS_GA_LOGOUT);
		channel->closeSession();
	});

	auth.start();
	game.start();
	test.addChannel(&auth);
	test.addChannel(&game);
	test.run();
}

// GS connect, connect to auth, account list, auth disconnect, client kick failed, auth connect
TEST(TS_GA_CLIENT_LOGIN, auth_disconnect_client_kick_failed_auth_connect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(
		    channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		EXPECT_EQ(1, packet->server_idx);
		EXPECT_STREQ("Server name", packet->server_name);
		EXPECT_STREQ("http://www.example.com/index.html", packet->server_screenshot_url);
		EXPECT_FALSE(packet->is_adult_server);
		EXPECT_STREQ("121.131.165.156", packet->server_ip);
		EXPECT_EQ(4516, packet->server_port);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		channel->closeSession();
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		TS_GA_CLIENT_KICK_FAILED clientKickPacket;
		TS_MESSAGE::initMessage(&clientKickPacket);
		strcpy(clientKickPacket.account, "account");
		game.sendPacket(&clientKickPacket);

		channel->start();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_CLIENT_KICK_FAILED* packet = AGET_PACKET(TS_GA_CLIENT_KICK_FAILED);
		EXPECT_STREQ("account", packet->account);

		game.closeSession();
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGOUT* packet = AGET_PACKET(TS_GA_LOGOUT);
		channel->closeSession();
	});

	auth.start();
	game.start();
	test.addChannel(&auth);
	test.addChannel(&game);
	test.run();
}

// GS connect, connect to auth, account list, client connection x200, auth disconnect, auth connect, 200 accounts in 2
// packets (195 + 5)
TEST(TS_GA_CLIENT_LOGIN, client_connect_x200_auth_reconnect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(
		    channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		EXPECT_EQ(1, packet->server_idx);
		EXPECT_STREQ("Server name", packet->server_name);
		EXPECT_STREQ("http://www.example.com/index.html", packet->server_screenshot_url);
		EXPECT_FALSE(packet->is_adult_server);
		EXPECT_STREQ("121.131.165.156", packet->server_ip);
		EXPECT_EQ(4516, packet->server_port);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		for(int i = 0; i < 200; i++) {
			TS_GA_CLIENT_LOGIN clientLoginPacket;
			TS_MESSAGE::initMessage(&clientLoginPacket);
			sprintf(clientLoginPacket.account, "account%d", i);
			clientLoginPacket.one_time_key = i;
			game.sendPacket(&clientLoginPacket);
		}
	});

	for(int i = 0; i < 200; i++) {
		auth.addCallback([i](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
			const TS_GA_CLIENT_LOGIN* packet = AGET_PACKET(TS_GA_CLIENT_LOGIN);
			char accountName[61];
			sprintf(accountName, "account%d", i);

			EXPECT_STREQ(accountName, packet->account);
			EXPECT_EQ(i, packet->one_time_key);

			TS_AG_CLIENT_LOGIN_EXTENDED clientLoginPacket;
			TS_MESSAGE::initMessage(&clientLoginPacket);
			strcpy(clientLoginPacket.account, accountName);
			clientLoginPacket.nAccountID = i;
			clientLoginPacket.result = TS_RESULT_SUCCESS;
			clientLoginPacket.ip = i + 1;
			clientLoginPacket.loginTime = i;
			channel->sendPacket(&clientLoginPacket);
		});

		game.addCallback([i, &auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
			const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);
			char accountName[61];
			sprintf(accountName, "account%d", i);

			ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
			EXPECT_EQ(i, packet->nAccountID);
			EXPECT_STREQ(accountName, packet->account);

			if(i == 199)
				auth.closeSession();
		});
	}

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		channel->start();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(195, packet->count);
		EXPECT_FALSE(packet->final_packet);

		for(int i = 0; i < packet->count; i++) {
			const TS_GA_ACCOUNT_LIST::AccountInfo* accountInfo = &packet->accountInfo[i];
			char accountName[61];
			sprintf(accountName, "account%d", i);

			EXPECT_STREQ(accountName, accountInfo->account);
			EXPECT_EQ(i + 1, accountInfo->ip);
			EXPECT_EQ(i, accountInfo->loginTime);
		}
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(5, packet->count);
		EXPECT_TRUE(packet->final_packet);

		for(int i = 0; i < packet->count; i++) {
			const TS_GA_ACCOUNT_LIST::AccountInfo* accountInfo = &packet->accountInfo[i];
			char accountName[61];
			sprintf(accountName, "account%d", i + 195);

			EXPECT_STREQ(accountName, accountInfo->account);
			EXPECT_EQ(i + 1 + 195, accountInfo->ip);
			EXPECT_EQ(i + 195, accountInfo->loginTime);
		}

		game.closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGOUT* packet = AGET_PACKET(TS_GA_LOGOUT);
		channel->closeSession();
	});

	auth.start();
	game.start();
	test.addChannel(&auth);
	test.addChannel(&game);
	test.run();
}

// GS connect, connect to auth, account list, client connection failed, auth disconnect, auth connect, account list
// empty
TEST(TS_GA_CLIENT_LOGIN, client_connect_failed) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(
		    channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		EXPECT_EQ(1, packet->server_idx);
		EXPECT_STREQ("Server name", packet->server_name);
		EXPECT_STREQ("http://www.example.com/index.html", packet->server_screenshot_url);
		EXPECT_FALSE(packet->is_adult_server);
		EXPECT_STREQ("121.131.165.156", packet->server_ip);
		EXPECT_EQ(4516, packet->server_port);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		TS_GA_CLIENT_LOGIN clientLoginPacket;
		TS_MESSAGE::initMessage(&clientLoginPacket);
		strcpy(clientLoginPacket.account, "account");
		clientLoginPacket.one_time_key = 42;
		game.sendPacket(&clientLoginPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_CLIENT_LOGIN* packet = AGET_PACKET(TS_GA_CLIENT_LOGIN);
		EXPECT_STREQ("account", packet->account);
		EXPECT_EQ(42, packet->one_time_key);

		TS_AG_CLIENT_LOGIN_EXTENDED clientLoginPacket;
		TS_MESSAGE::initMessage(&clientLoginPacket);
		strcpy(clientLoginPacket.account, "account");
		clientLoginPacket.nAccountID = 0;
		clientLoginPacket.result = TS_RESULT_ACCESS_DENIED;
		channel->sendPacket(&clientLoginPacket);
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		ASSERT_EQ(TS_RESULT_ACCESS_DENIED, packet->result);
		EXPECT_EQ(0, packet->nAccountID);
		EXPECT_STREQ("account", packet->account);

		auth.closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		channel->start();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT_EXT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT_EXT);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_SUCCESS;
		channel->sendPacket(&loginResult);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		game.closeSession();
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGOUT* packet = AGET_PACKET(TS_GA_LOGOUT);
		channel->closeSession();
	});

	auth.start();
	game.start();
	test.addChannel(&auth);
	test.addChannel(&game);
	test.run();
}

}  // namespace AuthServer
