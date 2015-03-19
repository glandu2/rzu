#include "gtest.h"
#include "RzTest.h"
#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_RSA_PUBLIC_KEY.h"
#include "Packets/TS_CA_SERVER_LIST.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_AC_RESULT_WITH_STRING.h"
#include "Packets/TS_AC_AES_KEY_IV.h"
#include "Packets/TS_AC_SERVER_LIST.h"

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

TEST(TS_CA_SERVER_LIST, empty_list) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAccountDES(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_EQ(TS_AC_RESULT::LSF_EULA_ACCEPTED, packet->login_flag);

		TS_CA_SERVER_LIST serverListPacket;
		TS_MESSAGE::initMessage(&serverListPacket);
		channel->sendPacket(&serverListPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_SERVER_LIST* packet = AGET_PACKET(TS_AC_SERVER_LIST);

		EXPECT_EQ(0, packet->count);

		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}
