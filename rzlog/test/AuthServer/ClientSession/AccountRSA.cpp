#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_RSA_PUBLIC_KEY.h"
#include "Packets/TS_AC_AES_KEY_IV.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/PacketEnums.h"
#include "Common.h"


/* TODO:
 * max size password (which must pass)
 */

namespace AuthServer {

class TS_CA_ACCOUNT_RSA_Test : public ::testing::Test {
public:
	virtual void SetUp() {
		rsaCipher = createRSAKey();
	}
	virtual void TearDown() {
		freeRSAKey(rsaCipher);
		rsaCipher = nullptr;
	}

	RSA* rsaCipher;
	unsigned char aes_key_iv[32];
};

TEST_F(TS_CA_ACCOUNT_RSA_Test, valid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		sendAccountRSA(aes_key_iv, channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		sendAccountRSA(aes_key_iv, channel, "invalidaccount", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_password) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		sendAccountRSA(aes_key_iv, channel, "test1", "invalid_password");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, rsa_key_invalid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		TS_CA_RSA_PUBLIC_KEY *keyMsg;
		int public_key_size = 256;

		keyMsg = TS_MESSAGE_WNA::create<TS_CA_RSA_PUBLIC_KEY, unsigned char>(public_key_size);
		keyMsg->key_size = public_key_size;
		memset(keyMsg->key, 127, public_key_size);
		channel->sendPacket(keyMsg);
		TS_MESSAGE_WNA::destroy(keyMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, rsa_key_size_too_large) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		TS_CA_RSA_PUBLIC_KEY *keyMsg;
		int public_key_size = 7;

		keyMsg = TS_MESSAGE_WNA::create<TS_CA_RSA_PUBLIC_KEY, unsigned char>(public_key_size);
		keyMsg->key_size = public_key_size*2;
		memset(keyMsg->key, 127, public_key_size);
		channel->sendPacket(keyMsg);
		TS_MESSAGE_WNA::destroy(keyMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, rsa_key_size_negative) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		TS_CA_RSA_PUBLIC_KEY *keyMsg;
		int public_key_size = 7;

		keyMsg = TS_MESSAGE_WNA::create<TS_CA_RSA_PUBLIC_KEY, unsigned char>(public_key_size);
		keyMsg->key_size = -public_key_size;
		memset(keyMsg->key, 127, public_key_size);
		channel->sendPacket(keyMsg);
		TS_MESSAGE_WNA::destroy(keyMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, garbage_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(aes_key_iv, &accountMsg, "test1", "admin");
		memset(accountMsg.account, 127, sizeof(accountMsg.account));

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, garbage_password_after_rsa) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(aes_key_iv, &accountMsg, "test1", "admin");
		memset(accountMsg.password, 127, sizeof(accountMsg.password));

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, long_password_47_chars) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(aes_key_iv, &accountMsg, "testPw47Chars", "47_chars_long_password_xxxxxxxxxxxxxxxxxxxxxxxx");

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, long_password_60_chars) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(aes_key_iv, &accountMsg, "testPw60Chars", "60_chars_long_password_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, garbage_rsa_data) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(aes_key_iv, &accountMsg, "test1", "60_chars_long_password_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
		accountMsg.password[55] = 127;
		accountMsg.password[56] = 127;

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_password_data_size) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(aes_key_iv, &accountMsg, "test1", "admin");
		accountMsg.password_size = 989;

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_password_data_size_negative) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		sendRSAKey(rsaCipher, channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(rsaCipher, packet, aes_key_iv);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(aes_key_iv, &accountMsg, "test1", "admin");
		accountMsg.password_size = -9;

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	auth.start();
	test.run();
}

} // namespace AuthServer
