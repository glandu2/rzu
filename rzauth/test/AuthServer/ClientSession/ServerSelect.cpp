#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/TS_AC_SERVER_LIST.h"
#include "Packets/TS_AC_SELECT_SERVER.h"
#include "Packets/TS_CA_SELECT_SERVER.h"
#include "Packets/TS_AG_CLIENT_LOGIN.h"
#include "Packets/PacketEnums.h"
#include "Common.h"
#include "../GameServerSession/Common.h"
#include "openssl/evp.h"

namespace AuthServer {

TEST(TS_CA_SELECT_SERVER, valid_aes) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	unsigned char aesKey[32];

	game.start();

	addGameLoginScenario(game,
						 11,
						 "Server 11",
						 "http://www.example.com/index_11.html",
						 false,
						 "127.0.0.1",
						 4514,
						 [&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test5", "admin", aesKey);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		TS_CA_SELECT_SERVER selectServerPkt;
		TS_MESSAGE::initMessage(&selectServerPkt);
		selectServerPkt.server_idx = 11;
		channel->sendPacket(&selectServerPkt);
	});

	auth.addCallback([&game, &aesKey](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		EVP_CIPHER_CTX d_ctx;
		int bytesWritten;

		const TS_AC_SELECT_SERVER_RSA* packet = AGET_PACKET(TS_AC_SELECT_SERVER_RSA);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_NE(0, packet->encrypted_data_size);
		union {
			uint64_t oneTimePassword;
			unsigned char raw[128];
		} otp;
		//pendingTime ignored here

		EVP_CIPHER_CTX_init(&d_ctx);
		ASSERT_NE(0, EVP_DecryptInit_ex(&d_ctx, EVP_aes_128_cbc(), NULL, aesKey, aesKey + 16));
		ASSERT_NE(0, EVP_DecryptInit_ex(&d_ctx, NULL, NULL, NULL, NULL));
		ASSERT_NE(0, EVP_DecryptUpdate(&d_ctx, otp.raw, &bytesWritten, packet->encrypted_data, sizeof(packet->encrypted_data)));
		ASSERT_NE(0, EVP_DecryptFinal_ex(&d_ctx, otp.raw + bytesWritten, &bytesWritten));
		EVP_CIPHER_CTX_cleanup(&d_ctx);

		channel->closeSession();
		sendClientLogin(&game, "test5", otp.oneTimePassword);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_STREQ("test5", packet->account);
		EXPECT_EQ(5, packet->nAccountID);
		EXPECT_EQ(0, packet->nPCBangUser);
		EXPECT_EQ(0, packet->nEventCode);
		EXPECT_EQ(19, packet->nAge);
//		EXPECT_EQ(0, packet->nContinuousPlayTime);
//		EXPECT_EQ(0, packet->nContinuousLogoutTime);

		sendClientLogout(channel, "test5");
		channel->closeSession();
	});

	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SELECT_SERVER, valid_des) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	unsigned char aesKey[32];

	game.start();

	addGameLoginScenario(game,
						 12,
						 "Server 12",
						 "http://www.example.com/index_12.html",
						 false,
						 "127.0.0.1",
						 4514,
						 [&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Des, "test6", "admin", nullptr);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		TS_CA_SELECT_SERVER selectServerPkt;
		TS_MESSAGE::initMessage(&selectServerPkt);
		selectServerPkt.server_idx = 12;
		channel->sendPacket(&selectServerPkt);
	});

	auth.addCallback([&game, &aesKey](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SELECT_SERVER* packet = AGET_PACKET(TS_AC_SELECT_SERVER);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		channel->closeSession();
		sendClientLogin(&game, "test6", packet->one_time_key);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_STREQ("test6", packet->account);
		EXPECT_EQ(6, packet->nAccountID);
		EXPECT_EQ(0, packet->nPCBangUser);
		EXPECT_EQ(0, packet->nEventCode);
		EXPECT_EQ(19, packet->nAge);
//		EXPECT_EQ(0, packet->nContinuousPlayTime);
//		EXPECT_EQ(0, packet->nContinuousLogoutTime);

		sendClientLogout(channel, "test6");
		channel->closeSession();
	});

	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SELECT_SERVER, invalid_index) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.start();

	addGameLoginScenario(game,
						 11,
						 "Server 11",
						 "http://www.example.com/index_11.html",
						 false,
						 "127.0.0.1",
						 4514,
						 [&auth](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		auth.start();
	});

	addClientLoginToServerListScenario(auth, AM_Aes, "test5", "admin");

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		TS_CA_SELECT_SERVER selectServerPkt;
		TS_MESSAGE::initMessage(&selectServerPkt);
		selectServerPkt.server_idx = 654;
		channel->sendPacket(&selectServerPkt);
	});

	auth.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);

		game.closeSession();
	});

	test.addChannel(&game);
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_SELECT_SERVER, not_authenticated) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);

		TS_CA_SELECT_SERVER selectServerPkt;
		TS_MESSAGE::initMessage(&selectServerPkt);
		selectServerPkt.server_idx = 654;
		channel->sendPacket(&selectServerPkt);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

} // namespace AuthServer
