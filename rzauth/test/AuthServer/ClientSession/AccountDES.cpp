#include "gtest.h"
#include "RzTest.h"
#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_RSA_PUBLIC_KEY.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_AC_RESULT_WITH_STRING.h"
#include "Packets/TS_AC_AES_KEY_IV.h"

#include "DesPasswordCipher.h"

static void sendVersion(TestConnectionChannel* channel) {
	TS_CA_VERSION version;
	TS_MESSAGE::initMessage(&version);
	strcpy(version.szVersion, "201501120");
	channel->sendPacket(&version);
}

static void sendAccountDES(TestConnectionChannel* channel, const char* account, const char* password) {
	TS_CA_ACCOUNT accountPacket;
	TS_MESSAGE::initMessage(&accountPacket);

	strcpy(accountPacket.account, account);
	strcpy(reinterpret_cast<char*>(accountPacket.password), password);
	DesPasswordCipher("MERONG").encrypt(accountPacket.password, sizeof(accountPacket.password));

	channel->sendPacket(&accountPacket);
}

static void expectAuthResult(TestConnectionChannel::Event& event, uint16_t result, int32_t login_flag) {
	ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type);
	EXPECT_TRUE(event.packet->id == TS_AC_RESULT::packetID || event.packet->id == TS_AC_RESULT_WITH_STRING::packetID);

	if(event.packet->id == TS_AC_RESULT::packetID) {
		const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

		EXPECT_EQ(result, packet->result);
		EXPECT_EQ(login_flag, packet->login_flag);
	} else if(event.packet->id == TS_AC_RESULT_WITH_STRING::packetID) {
		const TS_AC_RESULT_WITH_STRING* packet = AGET_PACKET(TS_AC_RESULT_WITH_STRING);

		if(result != TS_RESULT_SUCCESS)
			EXPECT_EQ(TS_RESULT_MISC, packet->result);
		else
			EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_EQ(login_flag, packet->login_flag);
	}
}

TEST(TS_CA_ACCOUNT_DES, valid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAccountDES(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, invalid_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAccountDES(channel, "invalidaccount", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, invalid_password) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAccountDES(channel, "test1", "invalid_password");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, garbage_data) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);

		TS_CA_ACCOUNT accountPacket;
		TS_MESSAGE::initMessage(&accountPacket);

		memset(accountPacket.account, 127, sizeof(accountPacket.account));
		memset(accountPacket.password, 127, sizeof(accountPacket.password));

		channel->sendPacket(&accountPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, garbage_data_before_des) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);

		TS_CA_ACCOUNT accountPacket;
		TS_MESSAGE::initMessage(&accountPacket);

		strcpy(accountPacket.account, "test1");
		memset(accountPacket.password, 127, sizeof(accountPacket.password));
		DesPasswordCipher("MERONG").encrypt(accountPacket.password, sizeof(accountPacket.password));

		channel->sendPacket(&accountPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}
