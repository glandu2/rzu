#include "Cipher/RzHashReversible256.h"
#include "Environment.h"
#include "GlobalConfig.h"
#include "RzTest.h"
#include "gtest/gtest.h"

#include "AuthClient/TS_AC_RESULT.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_CA_DISTRIBUTION_INFO.h"
#include "AuthClient/TS_CA_VERSION.h"

#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include "GameClient/TS_CS_REPORT.h"
#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_SC_RESULT.h"

TEST(Filter_Test, auth_to_gs_connection) {
	RzTest test;

	cval<std::string> gameClientListenIp;
	cval<int> gameClientListenPort;

	TestConnectionChannel authServer(
	    TestConnectionChannel::Server, CONFIG_GET()->server.ip, CONFIG_GET()->server.port, true);
	TestConnectionChannel authClient(
	    TestConnectionChannel::Client, CONFIG_GET()->client.ip, CONFIG_GET()->client.port, true);

	TestConnectionChannel gameServer(
	    TestConnectionChannel::Server, CONFIG_GET()->gameServer.ip, CONFIG_GET()->gameServer.port, true);
	TestConnectionChannel gameClient(TestConnectionChannel::Client, gameClientListenIp, gameClientListenPort, true);

	TS_CA_VERSION authVersionPacket;
	authVersionPacket.szVersion = "20190102359826482657";

	TS_AC_SERVER_LIST serverListPacket;
	serverListPacket.last_login_server_idx = 1;
	serverListPacket.servers.push_back(TS_SERVER_INFO());
	serverListPacket.servers.back().server_idx = 2;
	serverListPacket.servers.back().server_name = "Server idx 2";
	serverListPacket.servers.back().is_adult_server = false;
	serverListPacket.servers.back().server_screenshot_url = "http://localhost:82/";
	serverListPacket.servers.back().server_ip = CONFIG_GET()->gameServer.ip;
	serverListPacket.servers.back().server_port = CONFIG_GET()->gameServer.port;
	serverListPacket.servers.back().user_ratio = 32;

	serverListPacket.servers.push_back(TS_SERVER_INFO());
	serverListPacket.servers.back().server_idx = 1;
	serverListPacket.servers.back().server_name = "Server idx 1";
	serverListPacket.servers.back().is_adult_server = true;
	serverListPacket.servers.back().server_screenshot_url = "http://localhost:81/";
	serverListPacket.servers.back().server_ip = "127.0.0.3";
	serverListPacket.servers.back().server_port = CONFIG_GET()->gameServer.port + 2;
	serverListPacket.servers.back().user_ratio = 52;

	// First connection => ok
	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "auth client connected\n");
	});

	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "auth server connected\n");

		authClient.sendPacket(authVersionPacket, EPIC_LATEST);
	});

	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_CA_VERSION packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received auth version packet on server side\n");

		EXPECT_EQ("201507080", packet.szVersion);

		authServer.sendPacket(serverListPacket, EPIC_LATEST);
	});

	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_AC_SERVER_LIST packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received auth server list packet on client side\n");

		EXPECT_EQ(serverListPacket.last_login_server_idx, packet.last_login_server_idx);
		ASSERT_EQ(serverListPacket.servers.size(), packet.servers.size());

		EXPECT_EQ(serverListPacket.servers[0].server_idx, packet.servers[0].server_idx);
		EXPECT_EQ(serverListPacket.servers[0].server_name, packet.servers[0].server_name);
		EXPECT_EQ(serverListPacket.servers[0].is_adult_server, packet.servers[0].is_adult_server);
		EXPECT_EQ(serverListPacket.servers[0].server_screenshot_url, packet.servers[0].server_screenshot_url);
		EXPECT_EQ(serverListPacket.servers[0].user_ratio, packet.servers[0].user_ratio);

		EXPECT_EQ(std::string("127.0.0.1"), packet.servers[0].server_ip);

		EXPECT_EQ(serverListPacket.servers[1].server_idx, packet.servers[1].server_idx);
		EXPECT_EQ(serverListPacket.servers[1].server_name, packet.servers[1].server_name);
		EXPECT_EQ(serverListPacket.servers[1].is_adult_server, packet.servers[1].is_adult_server);
		EXPECT_EQ(serverListPacket.servers[1].server_screenshot_url, packet.servers[1].server_screenshot_url);
		EXPECT_EQ(serverListPacket.servers[1].user_ratio, packet.servers[1].user_ratio);

		EXPECT_EQ(std::string("127.0.0.1"), packet.servers[1].server_ip);

		gameClientListenIp = packet.servers[0].server_ip;
		gameClientListenPort = packet.servers[0].server_port;

		gameServer.start();
		gameClient.start();

		authClient.closeSession();
	});

	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "auth server disconnected\n");
		authServer.closeSession();
	});

	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "auth client disconnected\n");
	});

	// Game connection

	TS_CS_VERSION gameVersionPacket;
	gameVersionPacket.szVersion = "20180102359826482657";
	RzHashReversible256::generatePayload(gameVersionPacket);

	TS_SC_RESULT resultPacket;
	resultPacket.request_msg_id = 500;
	resultPacket.value = 0x87654321;
	resultPacket.result = 0x5678;

	gameClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "game client connected\n");
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "game server connected\n");

		gameClient.sendPacket(gameVersionPacket, EPIC_LATEST);
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_CS_VERSION packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received game version packet on server side\n");

		EXPECT_EQ("20200713", packet.szVersion);
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_CS_REPORT packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received game report packet on server side\n");

		gameServer.sendPacket(resultPacket, EPIC_LATEST);
	});

	gameClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_SC_RESULT packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received game result packet on client side\n");

		EXPECT_EQ(resultPacket.request_msg_id, packet.request_msg_id);
		EXPECT_EQ(resultPacket.value, packet.value);
		EXPECT_EQ(resultPacket.result, packet.result);

		gameClient.closeSession();
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "game server disconnected\n");
		gameServer.closeSession();
	});

	gameClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "game client disconnected\n");
	});

	test.addChannel(&authServer);
	authServer.start();
	test.addChannel(&authClient);
	authClient.start();
	test.run(5000);
}

TEST(Filter_Test, auth_to_gs_connection_no_close_in_lua_keep_server) {
	RzTest test;

	cval<std::string> gameClientListenIp;
	cval<int> gameClientListenPort;

	TestConnectionChannel authServer(
	    TestConnectionChannel::Server, CONFIG_GET()->server.ip, CONFIG_GET()->server.port, true);
	TestConnectionChannel authClient(
	    TestConnectionChannel::Client, CONFIG_GET()->client.ip, CONFIG_GET()->client.port, true);

	TestConnectionChannel gameServer(
	    TestConnectionChannel::Server, CONFIG_GET()->gameServer.ip, CONFIG_GET()->gameServer.port, true);
	TestConnectionChannel gameClient(TestConnectionChannel::Client, gameClientListenIp, gameClientListenPort, true);

	TS_CA_DISTRIBUTION_INFO authDistributionPacket;
	authDistributionPacket.distributionInfo = "NODC";

	TS_AC_SERVER_LIST serverListPacket;
	serverListPacket.last_login_server_idx = 1;
	serverListPacket.servers.push_back(TS_SERVER_INFO());
	serverListPacket.servers.back().server_idx = 2;
	serverListPacket.servers.back().server_name = "Server idx 2";
	serverListPacket.servers.back().is_adult_server = false;
	serverListPacket.servers.back().server_screenshot_url = "http://localhost:82/";
	serverListPacket.servers.back().server_ip = CONFIG_GET()->gameServer.ip;
	serverListPacket.servers.back().server_port = CONFIG_GET()->gameServer.port;
	serverListPacket.servers.back().user_ratio = 32;

	// Connecting client to rzfilter
	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "auth client connected\n");
	});

	// rzfilter receive client connection and connect to the auth server
	// send a packet from the client
	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "auth server connected, sending TS_CA_DISTRIBUTION_INFO packet\n");

		authClient.sendPacket(authDistributionPacket, EPIC_LATEST);
	});

	// the packet from the client should be received on the auth server side
	// send a packet with the GS list so rzfilter listen for GS connections
	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_CA_DISTRIBUTION_INFO packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info,
		         "received TS_CA_DISTRIBUTION_INFO packet on server side, sending GS list from server\n");

		EXPECT_EQ(authDistributionPacket.distributionInfo, packet.distributionInfo);

		authServer.sendPacket(serverListPacket, EPIC_LATEST);
	});

	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_AC_SERVER_LIST packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received GS list packet on client side, closing client connection\n");

		EXPECT_EQ(serverListPacket.last_login_server_idx, packet.last_login_server_idx);
		ASSERT_EQ(serverListPacket.servers.size(), packet.servers.size());

		EXPECT_EQ(serverListPacket.servers[0].server_idx, packet.servers[0].server_idx);
		EXPECT_EQ(serverListPacket.servers[0].server_name, packet.servers[0].server_name);
		EXPECT_EQ(serverListPacket.servers[0].is_adult_server, packet.servers[0].is_adult_server);
		EXPECT_EQ(serverListPacket.servers[0].server_screenshot_url, packet.servers[0].server_screenshot_url);
		EXPECT_EQ(serverListPacket.servers[0].user_ratio, packet.servers[0].user_ratio);

		EXPECT_EQ(std::string("127.0.0.1"), packet.servers[0].server_ip);

		gameClientListenIp = packet.servers[0].server_ip;
		gameClientListenPort = packet.servers[0].server_port;

		authClient.closeSession();
	});

	bool authServerManuallyDisconnected = false;
	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "auth client disconnected, sending TS_AC_RESULT from server side\n");

		// Sending packet from the server while not having the client connected should not crash the filter
		TS_AC_RESULT resultPacket;
		resultPacket.request_msg_id = 1;
		resultPacket.login_flag = 5;
		resultPacket.result = 4;
		authServer.sendPacket(resultPacket, EPIC_LATEST);

		authServer.addYield(
		    [&](TestConnectionChannel* channel) {
			    test.log(Object::LL_Info, "Waited 100ms, disconnecting auth server\n");
			    authServerManuallyDisconnected = true;
			    authServer.closeSession();
		    },
		    100);
	});

	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "auth server disconnected, starting game client and server\n");

		EXPECT_EQ(true, authServerManuallyDisconnected)
		    << "Auth server connection should not be disconnected when client disconnect";

		gameServer.start();
		gameClient.start();
	});

	// Game connection

	TS_CS_CHAT_REQUEST gameChatPacket;
	gameChatPacket.message = "NODC";

	TS_SC_RESULT resultPacket;
	resultPacket.request_msg_id = 500;
	resultPacket.value = 0x87654321;
	resultPacket.result = 0x5678;

	gameClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "game client connected\n");
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "game server connected, sending chat packet from client\n");

		gameClient.sendPacket(gameChatPacket, EPIC_LATEST);
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_CS_CHAT_REQUEST packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received game chat packet on server side, closing client connection\n");

		EXPECT_EQ(gameChatPacket.message, packet.message);

		gameClient.closeSession();
	});

	bool gameServerManuallyDisconnected = false;
	gameClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info,
		         "game client disconnected, sending result packet from server side (which should do nothing)\n");

		// Sending packet from the server while not having the client connected should not crash the filter
		gameServer.sendPacket(resultPacket, EPIC_LATEST);

		authServer.addYield(
		    [&](TestConnectionChannel* channel) {
			    test.log(Object::LL_Info, "waited 100ms, closing server connection\n");
			    gameServerManuallyDisconnected = true;
			    gameServer.closeSession();
		    },
		    100);
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "game server disconnected\n");

		EXPECT_EQ(true, gameServerManuallyDisconnected)
		    << "Game server connection should not be disconnected when client disconnect";
	});

	test.addChannel(&authServer);
	authServer.start();
	test.addChannel(&authClient);
	authClient.start();
	test.run(5000);
}

TEST(Filter_Test, auth_to_gs_connection_no_close_in_lua_keep_client) {
	RzTest test;

	cval<std::string> gameClientListenIp;
	cval<int> gameClientListenPort;

	TestConnectionChannel authServer(
	    TestConnectionChannel::Server, CONFIG_GET()->server.ip, CONFIG_GET()->server.port, true);
	TestConnectionChannel authClient(
	    TestConnectionChannel::Client, CONFIG_GET()->client.ip, CONFIG_GET()->client.port, true);

	TestConnectionChannel gameServer(
	    TestConnectionChannel::Server, CONFIG_GET()->gameServer.ip, CONFIG_GET()->gameServer.port, true);
	TestConnectionChannel gameClient(TestConnectionChannel::Client, gameClientListenIp, gameClientListenPort, true);

	TS_CA_DISTRIBUTION_INFO authDistributionPacket;
	authDistributionPacket.distributionInfo = "NODC";

	TS_AC_SERVER_LIST serverListPacket;
	serverListPacket.last_login_server_idx = 1;
	serverListPacket.servers.push_back(TS_SERVER_INFO());
	serverListPacket.servers.back().server_idx = 2;
	serverListPacket.servers.back().server_name = "Server idx 2";
	serverListPacket.servers.back().is_adult_server = false;
	serverListPacket.servers.back().server_screenshot_url = "http://localhost:82/";
	serverListPacket.servers.back().server_ip = CONFIG_GET()->gameServer.ip;
	serverListPacket.servers.back().server_port = CONFIG_GET()->gameServer.port;
	serverListPacket.servers.back().user_ratio = 32;

	// Connecting client to rzfilter
	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "auth client connected\n");
	});

	// rzfilter receive client connection and connect to the auth server
	// send a packet from the client
	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "auth server connected, sending TS_CA_DISTRIBUTION_INFO packet\n");

		authClient.sendPacket(authDistributionPacket, EPIC_LATEST);
	});

	// the packet from the client should be received on the auth server side
	// send a packet with the GS list so rzfilter listen for GS connections
	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_CA_DISTRIBUTION_INFO packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info,
		         "received TS_CA_DISTRIBUTION_INFO packet on server side, sending GS list from server\n");

		EXPECT_EQ(authDistributionPacket.distributionInfo, packet.distributionInfo);

		authServer.sendPacket(serverListPacket, EPIC_LATEST);
	});

	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_AC_SERVER_LIST packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received GS list packet on client side, closing server connection\n");

		EXPECT_EQ(serverListPacket.last_login_server_idx, packet.last_login_server_idx);
		ASSERT_EQ(serverListPacket.servers.size(), packet.servers.size());

		EXPECT_EQ(serverListPacket.servers[0].server_idx, packet.servers[0].server_idx);
		EXPECT_EQ(serverListPacket.servers[0].server_name, packet.servers[0].server_name);
		EXPECT_EQ(serverListPacket.servers[0].is_adult_server, packet.servers[0].is_adult_server);
		EXPECT_EQ(serverListPacket.servers[0].server_screenshot_url, packet.servers[0].server_screenshot_url);
		EXPECT_EQ(serverListPacket.servers[0].user_ratio, packet.servers[0].user_ratio);

		EXPECT_EQ(std::string("127.0.0.1"), packet.servers[0].server_ip);

		gameClientListenIp = packet.servers[0].server_ip;
		gameClientListenPort = packet.servers[0].server_port;

		authServer.closeSession();
	});

	authServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "auth server disconnected, sending TS_CA_VERSION from client side\n");

		// Sending packet from the client while not having the server connected should not crash the filter
		TS_CA_VERSION versionPacket;
		versionPacket.szVersion = "20190201";
		authClient.sendPacket(versionPacket, EPIC_LATEST);
	});

	authClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "auth client disconnected, starting game client and server\n");

		gameServer.start();
		gameClient.start();
	});

	// Game connection

	TS_CS_CHAT_REQUEST gameChatPacket;
	gameChatPacket.message = "NODC";

	gameClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "game client connected\n");
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		test.log(Object::LL_Info, "game server connected, sending chat packet from client\n");

		gameClient.sendPacket(gameChatPacket, EPIC_LATEST);
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		TS_CS_CHAT_REQUEST packet;
		DESERIALIZE_PACKET(packet, EPIC_LATEST);

		test.log(Object::LL_Info, "received game chat packet on server side, closing server connection\n");

		EXPECT_EQ(gameChatPacket.message, packet.message);

		gameServer.closeSession();
	});

	gameServer.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info,
		         "game server disconnected, sending result packet from client side (which should do nothing)\n");

		// Sending packet from the client while not having the server connected should not crash the filter
		TS_CS_VERSION versionPacket;
		versionPacket.szVersion = "20200713";
		RzHashReversible256::generatePayload(versionPacket);
		gameClient.sendPacket(versionPacket, EPIC_LATEST);
	});

	gameClient.addCallback([&](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		test.log(Object::LL_Info, "game client disconnected\n");
	});

	test.addChannel(&authServer);
	authServer.start();
	test.addChannel(&authClient);
	authClient.start();
	test.run(5000);
}
