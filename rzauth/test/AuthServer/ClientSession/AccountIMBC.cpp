#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/TS_CA_IMBC_ACCOUNT.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/PacketEnums.h"
#include "Common.h"

#include "DesPasswordCipher.h"

/*
 * Double login
 * Duplicate account (kick)
 */

namespace AuthServer {

TEST(TS_CA_IMBC_ACCOUNT, valid) {
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

TEST(TS_CA_IMBC_ACCOUNT, double_account_before_result) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_CLIENT_SIDE_ERROR, 0);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_IMBC_ACCOUNT, double_account_after_result) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_CLIENT_SIDE_ERROR, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_IMBC_ACCOUNT, duplicate_auth_account_connection) {
	RzTest test;
	TestConnectionChannel auth1(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel auth2(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth1.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
	});

	auth1.addCallback([&auth2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		auth2.start();
	});

	auth2.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
	});

	auth1.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		EXPECT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth2.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_ALREADY_EXIST, 0);
		channel->closeSession();
	});

	auth1.start();
	test.addChannel(&auth1);
	test.addChannel(&auth2);
	test.run();
}

TEST(TS_CA_IMBC_ACCOUNT, valid_disconect_before_result) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountIMBC(channel, "test1", "admin");
		channel->closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_IMBC_ACCOUNT, invalid_account) {
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

TEST(TS_CA_IMBC_ACCOUNT, invalid_password) {
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

TEST(TS_CA_IMBC_ACCOUNT, garbage_data) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);

		TS_CA_IMBC_ACCOUNT accountPacket;
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

} // namespace AuthServer
