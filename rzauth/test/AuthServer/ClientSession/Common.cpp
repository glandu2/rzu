#include "Common.h"
#include "Packets/TS_CA_VERSION.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_IMBC_ACCOUNT.h"
#include "Packets/TS_CA_SERVER_LIST.h"
#include "Packets/TS_CA_RSA_PUBLIC_KEY.h"
#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_AC_RESULT_WITH_STRING.h"
#include "DesPasswordCipher.h"
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace AuthServer {

void sendVersion(TestConnectionChannel* channel, const char* versionStr) {
	TS_CA_VERSION version;
	TS_MESSAGE::initMessage(&version);
	strcpy(version.szVersion, versionStr);
	channel->sendPacket(&version);
}

void sendAccountDES(TestConnectionChannel* channel, const char* account, const char* password) {
	TS_CA_ACCOUNT accountPacket;
	TS_MESSAGE::initMessage(&accountPacket);

	strcpy(accountPacket.account, account);
	strcpy(reinterpret_cast<char*>(accountPacket.password), password);
	DesPasswordCipher("MERONG").encrypt(accountPacket.password, sizeof(accountPacket.password));

	channel->sendPacket(&accountPacket);
}

void sendAccountIMBC(TestConnectionChannel* channel, const char* account, const char* password) {
	TS_CA_IMBC_ACCOUNT accountPacket;
	TS_MESSAGE::initMessage(&accountPacket);

	strcpy(accountPacket.account, account);
	strcpy(reinterpret_cast<char*>(accountPacket.password), password);

	channel->sendPacket(&accountPacket);
}

RSA* createRSAKey() {
	return RSA_generate_key(1024, 65537, NULL, NULL);
}

void freeRSAKey(RSA* rsaCipher) {
	RSA_free(rsaCipher);
}

void sendRSAKey(RSA* rsaCipher, TestConnectionChannel* channel) {
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

void parseAESKey(RSA* rsaCipher, const TS_AC_AES_KEY_IV* packet, unsigned char aes_key_iv[32]) {
	unsigned char decrypted_data[256];

	int data_size = RSA_private_decrypt(packet->data_size, (unsigned char*)packet->rsa_encrypted_data, decrypted_data, rsaCipher, RSA_PKCS1_PADDING);
	ASSERT_EQ(32, data_size);

	memcpy(aes_key_iv, decrypted_data, 32);
}

template<typename PacketType>
void prepareAccountRSAPacket(unsigned char aes_key_iv[32], PacketType *accountMsg, const char* account, const char* password) {
	EVP_CIPHER_CTX e_ctx;
	const unsigned char *key_data = aes_key_iv;
	const unsigned char *iv_data = aes_key_iv + 16;

	int len = (int)strlen(password);
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
}
template void prepareAccountRSAPacket<TS_CA_ACCOUNT_RSA>(unsigned char aes_key_iv[32], TS_CA_ACCOUNT_RSA *accountMsg, const char* account, const char* password);
template void prepareAccountRSAPacket<TS_CA_IMBC_ACCOUNT_RSA>(unsigned char aes_key_iv[32], TS_CA_IMBC_ACCOUNT_RSA *accountMsg, const char* account, const char* password);

void sendAccountRSA(unsigned char aes_key_iv[32], TestConnectionChannel* channel, const char* account, const char* password) {
	TS_CA_ACCOUNT_RSA accountMsg;
	prepareAccountRSAPacket(aes_key_iv, &accountMsg, account, password);
	accountMsg.dummy[0] = accountMsg.dummy[1] = accountMsg.dummy[2] = 0;
	accountMsg.unknown_00000100 = 0x00000100;
	channel->sendPacket(&accountMsg);
}

void sendAccountIMBC_RSA(unsigned char aes_key_iv[32], TestConnectionChannel* channel, const char* account, const char* password) {
	TS_CA_IMBC_ACCOUNT_RSA accountMsg;
	prepareAccountRSAPacket(aes_key_iv, &accountMsg, account, password);
	channel->sendPacket(&accountMsg);
}

void expectAuthResult(TestConnectionChannel::Event& event, uint16_t result, int32_t login_flag) {
	ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type);
	EXPECT_TRUE(event.packet->id == TS_AC_RESULT::packetID || event.packet->id == TS_AC_RESULT_WITH_STRING::packetID);

	if(event.packet->id == TS_AC_RESULT::packetID) {
		const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

		ASSERT_EQ(result, packet->result);
		EXPECT_EQ(login_flag, packet->login_flag);
	} else if(event.packet->id == TS_AC_RESULT_WITH_STRING::packetID) {
		const TS_AC_RESULT_WITH_STRING* packet = AGET_PACKET(TS_AC_RESULT_WITH_STRING);

		if(result != TS_RESULT_SUCCESS)
			ASSERT_EQ(TS_RESULT_MISC, packet->result);
		else
			ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_EQ(login_flag, packet->login_flag);
	}
}

void addClientLoginToServerListScenario(TestConnectionChannel& auth, AuthMethod method, const char* account, const char* password, unsigned char *aesKey, const char* version) {
	if(method == AM_Aes) {
		RSA* rsaCipher = createRSAKey();
		auth.addCallback([rsaCipher, version](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
			ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
			sendVersion(channel, version);
			sendRSAKey(rsaCipher, channel);
		});

		auth.addCallback([rsaCipher, aesKey, account, password](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
			const TS_AC_AES_KEY_IV* packet = AGET_PACKET(TS_AC_AES_KEY_IV);
			if(aesKey) {
				parseAESKey(rsaCipher, packet, aesKey);
				sendAccountRSA(aesKey, channel, account, password);
			} else {
				unsigned char aes_key_iv[32];
				parseAESKey(rsaCipher, packet, aes_key_iv);
				sendAccountRSA(aes_key_iv, channel, account, password);
			}
		});
	} else {
		auth.addCallback([account, password, version](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
			ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
			sendVersion(channel, version);
			sendAccountDES(channel, account, password);
		});
	}

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);

		TS_CA_SERVER_LIST serverListPacket;
		TS_MESSAGE::initMessage(&serverListPacket);
		channel->sendPacket(&serverListPacket);
	});
}

} //namespace AuthServer
