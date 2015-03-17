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

	void sendAccountRSA(TestConnectionChannel* channel, const char* account, const char* password) {
		TS_CA_ACCOUNT_RSA accountMsg;
		EVP_CIPHER_CTX e_ctx;
		const unsigned char *key_data = aes_key_iv;
		const unsigned char *iv_data = aes_key_iv + 16;

		int len = strlen(password);
		int p_len = len, f_len = 0;

		TS_MESSAGE::initMessage(&accountMsg);
		strcpy(accountMsg.account, account);
		memset(accountMsg.password, 0, sizeof(accountMsg.password));

		EVP_CIPHER_CTX_init(&e_ctx);

		ASSERT_NE(0, EVP_EncryptInit_ex(&e_ctx, EVP_aes_128_cbc(), NULL, key_data, iv_data));
		ASSERT_NE(0, EVP_EncryptInit_ex(&e_ctx, NULL, NULL, NULL, NULL));
		ASSERT_NE(0, EVP_EncryptUpdate(&e_ctx, accountMsg.password, &p_len, (const unsigned char*)password, len));
		ASSERT_NE(0, EVP_EncryptFinal_ex(&e_ctx, accountMsg.password+p_len, &f_len));

		EVP_CIPHER_CTX_cleanup(&e_ctx);

		accountMsg.password_size = p_len + f_len;
		accountMsg.dummy[0] = accountMsg.dummy[1] = accountMsg.dummy[2] = 0;
		accountMsg.unknown_00000100 = 0x00000100;

		channel->sendPacket(&accountMsg);
	}
};

TEST_F(TS_CA_ACCOUNT_RSA_Test, valid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, "auth", true);

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
		const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_EQ(TS_AC_RESULT::LSF_EULA_ACCEPTED, packet->login_flag);

		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, "auth", true);

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		sendVersion(channel);
		sendAESKey(channel);
	});

	auth.addCallback([this](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
		parseAESKey(packet);
		sendAccountRSA(channel, "invalid_account", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

		EXPECT_EQ(TS_RESULT_INVALID_PASSWORD, packet->result);
		EXPECT_EQ(0, packet->login_flag);

		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, invalid_password) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, "auth", true);

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
		const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

		EXPECT_EQ(TS_RESULT_INVALID_PASSWORD, packet->result);
		EXPECT_EQ(0, packet->login_flag);

		channel->closeSession();
	});

	test.addChannel(&auth);
	test.run();
}

TEST_F(TS_CA_ACCOUNT_RSA_Test, rsa_key_invalid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, "auth", true);

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
	TestConnectionChannel auth(TestConnectionChannel::Client, "auth", true);

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
	TestConnectionChannel auth(TestConnectionChannel::Client, "auth", true);

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

/* TODO:
 * non null terminated account
 * non null terminated password (before AES encrypt)
 * garbage password (not valid AES data)
 * max size password (which must pass)
 * invalid password size
 */
