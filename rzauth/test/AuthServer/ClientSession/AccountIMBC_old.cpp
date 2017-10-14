#include "../GlobalConfig.h"
#include "AuthClient/Flat/TS_AC_RESULT.h"
#include "AuthClient/Flat/TS_AC_RESULT_WITH_STRING.h"
#include "AuthClient/Flat/TS_CA_IMBC_ACCOUNT.h"
#include "Common.h"
#include "PacketEnums.h"
#include "RzTest.h"
#include "gtest/gtest.h"

#include "Cipher/DesPasswordCipher.h"

/*
 * Double login
 * Duplicate account (kick)
 */

namespace AuthServer {

TEST(TS_CA_IMBC_ACCOUNT_OLD, valid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_IMBC_ACCOUNT_OLD, invalid_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "invalidaccount", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_IMBC_ACCOUNT_OLD, invalid_password) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "test1", "invalid_password");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_IMBC_ACCOUNT_OLD, garbage_data) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);

		TS_CA_IMBC_ACCOUNT_OLD accountPacket;
		TS_MESSAGE::initMessage(&accountPacket);

		memset(accountPacket.account, 127, sizeof(accountPacket.account));
		memset(accountPacket.password, 127, sizeof(accountPacket.password));

		channel->sendPacket(&accountPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

}  // namespace AuthServer
