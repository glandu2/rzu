#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/PacketEnums.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Common.h"

#include "DesPasswordCipher.h"

namespace AuthServer {

TEST(TS_GA_CLIENT_LOGIN, unexpected_client) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 6, "Server name 10", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::sendClientLogin(channel, "unexpected", 0);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		EXPECT_EQ(TS_RESULT_ACCESS_DENIED, packet->result);
		EXPECT_STREQ("unexpected", packet->account);
		EXPECT_EQ(0, packet->nAccountID);
		EXPECT_EQ(0, packet->nPCBangUser);
		EXPECT_EQ(0, packet->nEventCode);
		EXPECT_EQ(0, packet->nAge);
		EXPECT_EQ(0, packet->nContinuousPlayTime);
		EXPECT_EQ(0, packet->nContinuousLogoutTime);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

TEST(TS_GA_CLIENT_LOGIN, long_account) {
	RzTest test;
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	addGameLoginScenario(game, 7, "Server name 10", "http://www.example.com/index10.html", true, "127.0.0.1", 4517, [](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::sendClientLogin(channel, "61_chars_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 'a');
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		EXPECT_EQ(TS_RESULT_ACCESS_DENIED, packet->result);
		EXPECT_STREQ("61_chars_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", packet->account);
		EXPECT_EQ(0, packet->nAccountID);
		EXPECT_EQ(0, packet->nPCBangUser);
		EXPECT_EQ(0, packet->nEventCode);
		EXPECT_EQ(0, packet->nAge);
		EXPECT_EQ(0, packet->nContinuousPlayTime);
		EXPECT_EQ(0, packet->nContinuousLogoutTime);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.run();
}

} // namespace AuthServer
