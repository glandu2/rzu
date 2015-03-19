#include "gtest.h"
#include "RzTest.h"
#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_RSA_PUBLIC_KEY.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_AC_RESULT_WITH_STRING.h"
#include "Packets/TS_AC_AES_KEY_IV.h"

#include "DesPasswordCipher.h"
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

/* TODO:
 * max size password (which must pass)
 */

class TS_CA_ACCOUNT_RSA_Test : public ::testing::Test {
public:
	virtual void SetUp() {
		rsaCipher = RSA_generate_key(1024, 65537, NULL, NULL);
	}
	virtual void TearDown() {
		RSA_free(rsaCipher);
		rsaCipher = nullptr;
	}

	RSA* rsaCipher;
	unsigned char aes_key_iv[32];

	void sendVersion(TestConnectionChannel* channel) {
		TS_CA_VERSION version;
		TS_MESSAGE::initMessage(&version);
		strcpy(version.szVersion, "201501120");
		channel->sendPacket(&version);
	}

	void sendAESKey(TestConnectionChannel* channel) {
		TS_CA_RSA_PUBLIC_KEY *keyMsg;
		int public_key_size;

		BIO * b = BIO_new(BIO_s_mem());
		PEM_write_bio_RSA_PUBKEY(b, rsaCipher);

		public_key_size = BIO_get_mem_data(b, NULL);
		keyMsg = TS_MESSAGE_WNA::create<TS_CA_RSA_PUBLIC_KEY, unsigned char>(public_key_size);

		keyMsg->key_size = public_key_size;

		BIO_read(b, keyMsg->key, public_key_size);
		BIO_free(b);

		channel->sendPacket(keyMsg);
		TS_MESSAGE_WNA::destroy(keyMsg);
	}

	void parseAESKey(const TS_AC_AES_KEY_IV* packet) {
		unsigned char decrypted_data[256];

		int data_size = RSA_private_decrypt(packet->data_size, (unsigned char*)packet->rsa_encrypted_data, decrypted_data, (RSA*)rsaCipher, RSA_PKCS1_PADDING);
		ASSERT_EQ(32, data_size);

		memcpy(aes_key_iv, decrypted_data, 32);
	}

	void prepareAccountRSAPacket(TS_CA_ACCOUNT_RSA* accountMsg, const char* account, const char* password) {
		EVP_CIPHER_CTX e_ctx;
		const unsigned char *key_data = aes_key_iv;
		const unsigned char *iv_data = aes_key_iv + 16;

		int len = strlen(password);
		int p_len = len, f_len = 0;

		TS_MESSAGE::initMessage(accountMsg);
		strcpy(accountMsg->account, account);
		memset(accountMsg->password, 0, sizeof(accountMsg->password));

		EVP_CIPHER_CTX_init(&e_ctx);

		ASSERT_NE(0, EVP_EncryptInit_ex(&e_ctx, EVP_aes_128_cbc(), NULL, key_data, iv_data));
		ASSERT_NE(0, EVP_EncryptInit_ex(&e_ctx, NULL, NULL, NULL, NULL));
		ASSERT_NE(0, EVP_EncryptUpdate(&e_ctx, accountMsg->password, &p_len, (const unsigned char*)password, len));
		ASSERT_NE(0, EVP_EncryptFinal_ex(&e_ctx, accountMsg->password+p_len, &f_len));

		EVP_CIPHER_CTX_cleanup(&e_ctx);

		accountMsg->password_size = p_len + f_len;
		accountMsg->dummy[0] = accountMsg->dummy[1] = accountMsg->dummy[2] = 0;
		accountMsg->unknown_00000100 = 0x00000100;
	}

	void sendAccountRSA(TestConnectionChannel* channel, const char* account, const char* password) {
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, account, password);
		channel->sendPacket(&accountMsg);
	}
};

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

TEST_F(TS_CA_ACCOUNT_RSA_Test, valid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		sendAccountRSA(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		sendAccountRSA(channel, "invalidaccount", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_password) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		sendAccountRSA(channel, "test1", "invalid_password");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, rsa_key_invalid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
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
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, rsa_key_size_too_large) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
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
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, rsa_key_size_negative) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
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
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, garbage_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, "test1", "admin");
		memset(accountMsg.account, 127, sizeof(accountMsg.account));

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, garbage_password_after_rsa) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, "test1", "admin");
		memset(accountMsg.password, 127, sizeof(accountMsg.password));

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, long_password_47_chars) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, "testPw47Chars", "47_chars_long_password_xxxxxxxxxxxxxxxxxxxxxxxx");

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, long_password_60_chars) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, "testPw60Chars", "60_chars_long_password_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, garbage_rsa_data) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, "test1", "60_chars_long_password_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
		accountMsg.password[55] = 127;
		accountMsg.password[56] = 127;

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_password_data_size) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, "test1", "admin");
		accountMsg.password_size = 989;

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_password_data_size_negative) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		TS_CA_ACCOUNT_RSA accountMsg;
		prepareAccountRSAPacket(&accountMsg, "test1", "admin");
		accountMsg.password_size = -9;

		channel->sendPacket(&accountMsg);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
	});

	test.addChannel(&auth);
	test.run();
}
