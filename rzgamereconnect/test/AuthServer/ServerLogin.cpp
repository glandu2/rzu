#include "gtest/gtest.h"
#include "RzTest.h"
#include "GlobalConfig.h"
#include "PacketEnums.h"
#include "AuthGame/TS_AG_LOGIN_RESULT.h"
#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "AuthGame/TS_GA_LOGIN.h"
#include "AuthGame/TS_GA_LOGOUT.h"
#include "AuthGame/TS_GA_CLIENT_LOGOUT.h"
#include "AuthGame/TS_GA_ACCOUNT_LIST.h"
#include "Common.h"

#include "Cipher/DesPasswordCipher.h"

namespace AuthServer {

//GS connect, connect to auth failure, GS disconnection
TEST(TS_GA_LOGIN, gs_connect__auth_fail__gs_disconnect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	//GS connection
	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
		channel->closeSession();
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

//GS connect, connect to auth failure, client login (refused), GS disconnection
TEST(TS_GA_LOGIN, gs_connect__auth_fail__client_login__gs_disconnect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	//GS connection
	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
		AuthServer::sendClientLogin(channel, "test1", 0);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);
		ASSERT_STREQ("test1", packet->account);
		EXPECT_EQ(TS_RESULT_ACCESS_DENIED, packet->result);
		channel->closeSession();
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

//GS connect, connect to auth, GS disconnection, account list, GS logout
TEST(TS_GA_LOGIN, gs_connect__auth_connect__gs_disconnect__accout_list) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	//GS connection
	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		game.closeSession();
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT);

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

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGOUT* packet = AGET_PACKET(TS_GA_LOGOUT);
		channel->closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.start();
	game.start();
	test.addChannel(&auth);
	test.addChannel(&game);
	test.run();
}

//GS connect, connect to auth, account list, GS disconnection
TEST(TS_GA_LOGIN, gs_connect__auth_connect__account_list__gs_disconnect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT);

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

//GS connect, connect to auth, auth disconnection, GS disconnection, auth connection
TEST(TS_GA_LOGIN, gs_connect__auth_connect__auth_disconnect__gs_disconnect__auth_connect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);
	bool otherChannelDisconnected = false;

	//GS connection
	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	//Expect auth connection
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	//Auth GS login
	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT);

		EXPECT_EQ(1, packet->server_idx);
		EXPECT_STREQ("Server name", packet->server_name);
		EXPECT_STREQ("http://www.example.com/index.html", packet->server_screenshot_url);
		EXPECT_FALSE(packet->is_adult_server);
		EXPECT_STREQ("121.131.165.156", packet->server_ip);
		EXPECT_EQ(4516, packet->server_port);

		channel->closeSession();
		game.closeSession();
	});

	//auth disconnected
	auth.addCallback([&auth, &otherChannelDisconnected](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		if(otherChannelDisconnected)
			auth.start();
		else
			otherChannelDisconnected = true;
	});

	//GS disconnected
	game.addCallback([&auth, &otherChannelDisconnected](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		if(otherChannelDisconnected)
			auth.start();
		else
			otherChannelDisconnected = true;
	});

	//Expect auth connection
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	//Auth GS login
	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT);

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

	//Auth receive empty account list
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);
	});

	//Expect GS logout (clean logout even if GS disconnected when auth was offline)
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

//GS connect, connect to auth, account list, auth disconnection, GS disconnection, auth connection
TEST(TS_GA_LOGIN, gs_connect__auth_connect__account_list__auth_disconnect__gs_disconnect__auth_connect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	//GS connection
	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	//Expect auth connection
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	//Auth GS login
	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT);

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

	//GS receive the result
	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
	});

	//Auth receive empty account list
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);

		channel->closeSession();
	});

	//Auth is disconnected (simulate broken link)
	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		game.closeSession();
	});

	//When auth is fully closed, close GS
	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		auth.start();
	});

	// Auth reconnection, check GS auto login
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});
	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT);

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

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_ACCOUNT_LIST* packet = AGET_PACKET(TS_GA_ACCOUNT_LIST);
		EXPECT_EQ(0, packet->count);
		EXPECT_TRUE(packet->final_packet);
	});

	//Expect GS logout (clean logout even if GS disconnected when auth was offline)
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

//GS connect, connect to auth, gs login fail, GS disconnection
TEST(TS_GA_LOGIN, gs_connect__auth_connect__gs_login_fail__account_list__gs_disconnect) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel auth(TestConnectionChannel::Server, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, 1, "Server name", "http://www.example.com/index.html", false, "121.131.165.156", 4516);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_GA_LOGIN_WITH_LOGOUT* packet = AGET_PACKET(TS_GA_LOGIN_WITH_LOGOUT);

		EXPECT_EQ(1, packet->server_idx);
		EXPECT_STREQ("Server name", packet->server_name);
		EXPECT_STREQ("http://www.example.com/index.html", packet->server_screenshot_url);
		EXPECT_FALSE(packet->is_adult_server);
		EXPECT_STREQ("121.131.165.156", packet->server_ip);
		EXPECT_EQ(4516, packet->server_port);

		TS_AG_LOGIN_RESULT loginResult;
		TS_MESSAGE::initMessage(&loginResult);
		loginResult.result = TS_RESULT_ACCESS_DENIED;
		channel->sendPacket(&loginResult);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_ACCESS_DENIED, packet->result);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		game.closeSession();
	});

	game.addCallback([&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		auth.closeSession();
	});

	auth.start();
	game.start();
	test.addChannel(&auth);
	test.addChannel(&game);
	test.run();
}

} // namespace AuthServer
