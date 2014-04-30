#include "ClientSession.h"
#include "GameServerSession.h"
#include "../GlobalConfig.h"

#include <string.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>

#include "DesPasswordCipher.h"
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "DB_Account.h"

#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_AC_AES_KEY_IV.h"
#include "Packets/TS_AC_SELECT_SERVER.h"
#include "Packets/TS_AC_SERVER_LIST.h"

namespace AuthServer {

ClientSession::ClientSession() : RappelzSession(EncryptedSocket::Encrypted), useRsaAuth(false), lastLoginServerId(0), clientData(nullptr), dbQuery(nullptr) {
	addPacketsToListen(5,
					   TS_CA_VERSION::packetID,
					   TS_CA_RSA_PUBLIC_KEY::packetID,
					   TS_CA_ACCOUNT::packetID,
					   TS_CA_SERVER_LIST::packetID,
					   TS_CA_SELECT_SERVER::packetID
					   );
}

ClientSession::~ClientSession() {
	if(clientData)
		ClientData::removeClient(clientData);
	if(dbQuery)
		dbQuery->cancel();
}

void ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_CA_VERSION::packetID:
			onVersion(static_cast<const TS_CA_VERSION*>(packet));
			break;

		case TS_CA_RSA_PUBLIC_KEY::packetID:
			onRsaKey(static_cast<const TS_CA_RSA_PUBLIC_KEY*>(packet));
			break;

		case TS_CA_ACCOUNT::packetID:
			onAccount(static_cast<const TS_CA_ACCOUNT*>(packet));
			break;

		case TS_CA_SERVER_LIST::packetID:
			onServerList(static_cast<const TS_CA_SERVER_LIST*>(packet));
			break;

		case TS_CA_SELECT_SERVER::packetID:
			onSelectServer(static_cast<const TS_CA_SELECT_SERVER*>(packet));
			break;
	}
}

void ClientSession::onVersion(const TS_CA_VERSION* packet) {
	if(!memcmp(packet->szVersion, "TEST", 4)) {
		uint32_t totalUserCount = ClientData::getClientCount();
		TS_SC_RESULT result;
		TS_MESSAGE::initMessage<TS_SC_RESULT>(&result);

		result.value = totalUserCount ^ 0xADADADAD;
		result.result = 0;
		result.request_msg_id = packet->id;
		sendPacket(&result);
	}
}

void ClientSession::onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet) {
	TS_AC_AES_KEY_IV* aesKeyMessage = nullptr;
	RSA* rsaCipher = nullptr;
	BIO* bio = nullptr;
	int blockSize;

	for(int i = 0; i < 32; i++)
		aesKey[i] = rand() & 0xFF;

	const int expectedKeySize = packet->size - sizeof(TS_CA_RSA_PUBLIC_KEY);

	if(packet->key_size != expectedKeySize) {
		warn("RSA: key_size is invalid: %d, expected (from msg size): %d\n", packet->key_size, expectedKeySize);
		abortSession();
		goto cleanup;
	}

	bio = BIO_new_mem_buf((void*)packet->key, packet->key_size);
	rsaCipher = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
	if(rsaCipher == nullptr) {
		warn("RSA: invalid certificate\n");
		abortSession();
		goto cleanup;
	}

	aesKeyMessage = TS_MESSAGE_WNA::create<TS_AC_AES_KEY_IV, unsigned char>(RSA_size(rsaCipher));

	blockSize = RSA_public_encrypt(32, aesKey, aesKeyMessage->rsa_encrypted_data, rsaCipher, RSA_PKCS1_PADDING);
	if(blockSize < 0) {
		const char* errorString = ERR_error_string(ERR_get_error(), nullptr);
		warn("RSA: encrypt error: %s\n", errorString);
		abortSession();
		goto cleanup;
	}

	aesKeyMessage->data_size = blockSize;

	useRsaAuth = true;
	sendPacket(aesKeyMessage);

cleanup:
	if(aesKeyMessage)
		TS_MESSAGE_WNA::destroy(aesKeyMessage);
	if(bio)
		BIO_free(bio);
	if(rsaCipher)
		RSA_free(rsaCipher);
}

void ClientSession::onAccount(const TS_CA_ACCOUNT* packet) {
	unsigned char password[64];  //size = at most rsa encrypted size
	std::string account;
	bool ok = false;

	if(dbQuery != nullptr) {
		TS_AC_RESULT result;
		TS_MESSAGE::initMessage<TS_AC_RESULT>(&result);
		result.request_msg_id = TS_CA_ACCOUNT::packetID;
		result.result = TS_RESULT_CLIENT_SIDE_ERROR;
		result.login_flag = 0;
		sendPacket(&result);
		info("Client connection with a auth request already in progress\n");
		return;
	}

	if(useRsaAuth) {
		const TS_CA_ACCOUNT_V2* accountv2 = reinterpret_cast<const TS_CA_ACCOUNT_V2*>(packet);
		EVP_CIPHER_CTX d_ctx;
		int bytesWritten, totalLength;
		unsigned int bytesRead;

		debug("Client login using AES\n");

		account = std::string(accountv2->account, std::find(accountv2->account, accountv2->account + 60, '\0'));

		if(accountv2->aes_block_size > sizeof(password) - 16) {
			warn("RSA: invalid password length: %d\n", accountv2->aes_block_size);
		}

		EVP_CIPHER_CTX_init(&d_ctx);

		if(EVP_DecryptInit_ex(&d_ctx, EVP_aes_128_cbc(), NULL, aesKey, aesKey + 16) < 0)
			goto cleanup_aes;
		if(EVP_DecryptInit_ex(&d_ctx, NULL, NULL, NULL, NULL) < 0)
			goto cleanup_aes;

		for(totalLength = bytesRead = 0; bytesRead + 15 < accountv2->aes_block_size; bytesRead += 16) {
			if(EVP_DecryptUpdate(&d_ctx, password + totalLength, &bytesWritten, accountv2->password + bytesRead, 16) < 0)
				goto cleanup_aes;
			totalLength += bytesWritten;
		}

		if(EVP_DecryptFinal_ex(&d_ctx, password + totalLength, &bytesWritten) < 0)
			goto cleanup_aes;

		totalLength += bytesWritten;

		if(totalLength >= (int)sizeof(password))
			goto cleanup_aes;

		password[totalLength] = 0;
		ok = true;

	cleanup_aes:
		EVP_CIPHER_CTX_cleanup(&d_ctx);
	} else {
		std::string key = CONFIG_GET()->auth.client.desKey.get();
		debug("Client login using DES, key: %s\n", key.c_str());

		account = std::string(packet->account, std::find(packet->account, packet->account + 60, '\0'));

		memcpy((char*)password, packet->password, 61);
		DesPasswordCipher(key.c_str()).decrypt(password, 61);
		password[60] = 0;
		ok = true;
	}

	if(ok) {
		debug("Login request for account %s\n", account.c_str());

		dbQuery = new DB_Account(this, account, (char*)password, strlen((char*)password));
	} else {
		warn("Invalid Account message data from %s@%s\n", account.c_str(), getSocket()->getHost().c_str());
	}
}

void ClientSession::clientAuthResult(bool authOk, const std::string& account, uint32_t accountId, uint32_t age, uint16_t lastLoginServerIdx, uint32_t eventCode) {
	TS_AC_RESULT result;
	TS_MESSAGE::initMessage<TS_AC_RESULT>(&result);
	result.request_msg_id = TS_CA_ACCOUNT::packetID;

	dbQuery = nullptr;

	if(authOk == false) {
		result.result = TS_RESULT_INVALID_PASSWORD;
		result.login_flag = 0;
	} else if(clientData != nullptr) { //already connected
		result.result = TS_RESULT_CLIENT_SIDE_ERROR;
		result.login_flag = 0;
		info("Client connection already authenticated with account %s\n", clientData->account.c_str());
	} else {
		clientData = ClientData::tryAddClient(this, account, accountId, age, eventCode);
		if(clientData == nullptr) {
			result.result = TS_RESULT_ALREADY_EXIST;
			result.login_flag = 0;
			info("Client already connected\n");
		} else {
			result.result = 0;
			result.login_flag = TS_AC_RESULT::LSF_EULA_ACCEPTED;
			this->lastLoginServerId = lastLoginServerIdx;
		}
	}

	sendPacket(&result);
}

void ClientSession::onServerList(const TS_CA_SERVER_LIST* packet) {
	TS_AC_SERVER_LIST* serverListPacket;
	unsigned int i, j, count;

	// Check if user authenticated
	if(clientData == nullptr) {
		abortSession();
		return;
	}

	const std::vector<GameServerSession*>& serverList = GameServerSession::getServerList();

	for(i = count = 0; i < serverList.size(); i++)
		if(serverList.at(i) != nullptr)
			count++;

	serverListPacket = TS_MESSAGE_WNA::create<TS_AC_SERVER_LIST, TS_AC_SERVER_LIST::TS_SERVER_INFO>(count);

	serverListPacket->count = count;
	serverListPacket->last_login_server_idx = lastLoginServerId;

	for(i = j = 0; i < serverList.size() && j < serverListPacket->count; i++) {
		GameServerSession* serverInfo = serverList.at(i);

		if(serverInfo == nullptr)
			continue;

		serverListPacket->servers[j].server_idx = serverInfo->getServerIdx();
		strcpy(serverListPacket->servers[j].server_ip, serverInfo->getServerIp().c_str());
		serverListPacket->servers[j].server_port = serverInfo->getServerPort();
		strcpy(serverListPacket->servers[j].server_name, serverInfo->getServerName().c_str());
		serverListPacket->servers[j].is_adult_server = serverInfo->getIsAdultServer();
		strcpy(serverListPacket->servers[j].server_screenshot_url, serverInfo->getServerScreenshotUrl().c_str());
		serverListPacket->servers[j].user_ratio = 0;

		j++;
	}

	sendPacket(serverListPacket);
	TS_MESSAGE_WNA::destroy(serverListPacket);
}

void ClientSession::onSelectServer(const TS_CA_SELECT_SERVER* packet) {
	const std::vector<GameServerSession*>& serverList = GameServerSession::getServerList();

	if(clientData == nullptr) {
		abortSession();
		return;
	}

	if(packet->server_idx < serverList.size() && serverList.at(packet->server_idx) != nullptr) {
		GameServerSession* server = serverList.at(packet->server_idx);

		uint64_t oneTimePassword = (uint64_t)rand()*rand()*rand()*rand();

		//clientData now managed by target GS
		clientData->switchClientToServer(server, oneTimePassword);
		clientData = nullptr;

		if(useRsaAuth) {
			TS_AC_SELECT_SERVER_V2 result;
			TS_MESSAGE::initMessage<TS_AC_SELECT_SERVER_V2>(&result);
			result.result = 0;
			result.encrypted_data_size = 16;
			result.pending_time = 0;
			result.unknown = 0;
			result.unknown2 = 0;

			EVP_CIPHER_CTX e_ctx;
			int bytesWritten;
			bool ok = false;

			EVP_CIPHER_CTX_init(&e_ctx);
			if(EVP_EncryptInit_ex(&e_ctx, EVP_aes_128_cbc(), NULL, aesKey, aesKey + 16) < 0)
				goto cleanup;

			if(EVP_EncryptInit_ex(&e_ctx, NULL, NULL, NULL, NULL) < 0)
				goto cleanup;
			if(EVP_EncryptUpdate(&e_ctx, result.encrypted_data, &bytesWritten, (const unsigned char*)&oneTimePassword, sizeof(uint64_t)) < 0)
				goto cleanup;
			if(EVP_EncryptFinal_ex(&e_ctx, result.encrypted_data + bytesWritten, &bytesWritten) < 0)
				goto cleanup;

			sendPacket(&result);
			ok = true;

		cleanup:
			EVP_CIPHER_CTX_cleanup(&e_ctx);

			if(!ok)
				abortSession();
		} else {
			TS_AC_SELECT_SERVER result;
			TS_MESSAGE::initMessage<TS_AC_SELECT_SERVER>(&result);

			result.result = 0;
			result.one_time_key = oneTimePassword;
			result.pending_time = 0;

			sendPacket(&result);
		}
	} else {
		abortSession();
	}
}

} // namespace AuthServer
