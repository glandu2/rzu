#include "ClientSession.h"
#include "GameData.h"
#include "../GlobalConfig.h"
#include "rzauthGitVersion.h"

#include <string.h>
#include <stdlib.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "DB_Account.h"
#include "DB_UpdateLastServerIdx.h"

#include "LogServerClient.h"

#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_AC_AES_KEY_IV.h"
#include "Packets/TS_AC_SELECT_SERVER.h"
#include "Packets/TS_AC_SERVER_LIST.h"

#include "MessageBuffer.h"

namespace AuthServer {

ClientSession::ClientSession()
	: useRsaAuth(false),
	  version(EPIC_8_1),
	  lastLoginServerId(1),
	  serverIdxOffset(0),
	  clientData(nullptr),
	  dbQuery(nullptr)
{
}

ClientSession::~ClientSession() {
	if(clientData)
		ClientData::removeClient(clientData);
	if(dbQuery)
		dbQuery->cancel();
}

void ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	detectClientVersion(packet);

	MessageBuffer buffer(packet, version);

	switch(packet->id) {
		case TS_CA_VERSION::packetID:
			process<TS_CA_VERSION>(&buffer, &ClientSession::onVersion);
			break;

		case TS_CA_RSA_PUBLIC_KEY::packetID:
			process<TS_CA_RSA_PUBLIC_KEY>(&buffer, &ClientSession::onRsaKey);
			break;

		case TS_CA_ACCOUNT::packetID:
			process<TS_CA_ACCOUNT>(&buffer, &ClientSession::onAccount);
			break;

		case TS_CA_IMBC_ACCOUNT::packetID:
			process<TS_CA_IMBC_ACCOUNT>(&buffer, &ClientSession::onImbcAccount);
			break;

		case TS_CA_SERVER_LIST::packetID:
			process<TS_CA_SERVER_LIST>(&buffer, &ClientSession::onServerList);
			break;

		case TS_CA_SELECT_SERVER::packetID:
			process<TS_CA_SELECT_SERVER>(&buffer, &ClientSession::onSelectServer);
			break;

		default:
			debug("Unknown packet ID: %d, size: %d\n", packet->id, packet->size);
			break;
	}
}

void ClientSession::detectClientVersion(const TS_MESSAGE* packet) {
	if(packet->id == TS_CA_RSA_PUBLIC_KEY::packetID) {
		if(version < EPIC_8_1_1_RSA)
			version = EPIC_8_1_1_RSA;
	} else if(packet->id == TS_CA_ACCOUNT::packetID) {
		static TS_CA_ACCOUNT sizeDetermination;
		static const int epic4AccountPacketSize = sizeDetermination.getSize(EPIC_4_1);
		if(packet->size == epic4AccountPacketSize) {
			debug("Client is epic 4 or older\n");

			if(version >= EPIC_5_1)
				version = EPIC_4_1;
		}
	}
}

void ClientSession::onVersion(const TS_CA_VERSION& packet) {
	if(!memcmp(packet.szVersion, "TEST", 4)) {
		uint32_t totalUserCount = ClientData::getClientCount();
		TS_SC_RESULT result;
		TS_MESSAGE::initMessage<TS_SC_RESULT>(&result);

		result.value = totalUserCount ^ 0xADADADAD;
		result.result = 0;
		result.request_msg_id = packet.id;
		sendPacket(&result);
	} else if(!memcmp(packet.szVersion, "INFO", 4)) {
		static uint32_t gitVersionSuffix = 0;
		TS_SC_RESULT result;
		TS_MESSAGE::initMessage<TS_SC_RESULT>(&result);

		if(gitVersionSuffix == 0) {
			std::string shaPart(rzauthVersion+8, 8);
			gitVersionSuffix = strtoul(shaPart.c_str(), nullptr, 16);
		}

		result.value = gitVersionSuffix ^ 0xADADADAD;
		result.result = 0;
		result.request_msg_id = packet.id;
		sendPacket(&result);
	} else if(!memcmp(packet.szVersion, "200609280", 9) || !memcmp(packet.szVersion, "Creer", 5)) {
		version = EPIC_2_1;
	}
}

void ClientSession::onRsaKey(const TS_CA_RSA_PUBLIC_KEY& packet) {
	TS_AC_AES_KEY_IV aesKeyMessage;
	RSA* rsaCipher = nullptr;
	BIO* bio = nullptr;
	int blockSize;

	for(int i = 0; i < 32; i++)
		aesKey[i] = rand() & 0xFF;

	// ->size reference packet full size (need to change base struct)
	/*const int expectedKeySize = packet->size - sizeof(TS_CA_RSA_PUBLIC_KEY);

	if(packet->key.size() != expectedKeySize) {
		warn("RSA: key_size is invalid: %d, expected (from msg size): %d\n", packet->key_size, expectedKeySize);
		abortSession();
		goto cleanup;
	}*/

	ERR_clear_error();

	bio = BIO_new_mem_buf((void*)&packet.key[0], packet.key.size());
	rsaCipher = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
	if(rsaCipher == nullptr) {
		const char* errorString = ERR_error_string(ERR_get_error(), nullptr);
		warn("RSA: invalid certificate: %s\n", errorString);
		abortSession();
		goto cleanup;
	}

	aesKeyMessage.rsa_encrypted_data.resize(RSA_size(rsaCipher));

	blockSize = RSA_public_encrypt(32, aesKey, &aesKeyMessage.rsa_encrypted_data[0], rsaCipher, RSA_PKCS1_PADDING);
	if(blockSize < 0) {
		const char* errorString = ERR_error_string(ERR_get_error(), nullptr);
		warn("RSA: encrypt error: %s\n", errorString);
		abortSession();
		goto cleanup;
	}

	aesKeyMessage.rsa_encrypted_data.resize(blockSize);

	useRsaAuth = true;
	sendPacket(aesKeyMessage, version);

cleanup:
	if(bio)
		BIO_free(bio);
	if(rsaCipher)
		RSA_free(rsaCipher);
}

void ClientSession::onAccount(const TS_CA_ACCOUNT& packet) {
	std::string account;
	std::vector<unsigned char> cryptedPassword;

	if(dbQuery != nullptr) {
		TS_AC_RESULT result;
		result.request_msg_id = TS_CA_ACCOUNT::packetID;
		result.result = TS_RESULT_CLIENT_SIDE_ERROR;
		result.login_flag = 0;
		sendPacket(result, version);
		info("Client connection with a auth request already in progress\n");
		return;
	}

	account = Utils::convertToString(packet.account, sizeof(packet.account)-1);
	cryptedPassword = Utils::convertToDataArray(packet.password, sizeof(packet.password), packet.password_size);

	debug("Login request for account %s\n", account.c_str());

	dbQuery = new DB_Account(this, account, getStream()->getRemoteIpStr(), useRsaAuth ? DB_Account::EM_AES : DB_Account::EM_DES, cryptedPassword, aesKey);
}

void ClientSession::onImbcAccount(const TS_CA_IMBC_ACCOUNT& packet) {
	std::string account;
	std::vector<unsigned char> cryptedPassword;

	if(dbQuery != nullptr) {
		TS_AC_RESULT result;
		result.request_msg_id = TS_CA_ACCOUNT::packetID;
		result.result = TS_RESULT_CLIENT_SIDE_ERROR;
		result.login_flag = 0;
		sendPacket(result, version);
		info("Client IMBC connection with a auth request already in progress\n");
		return;
	}

	account = Utils::convertToString(packet.account, sizeof(packet.account)-1);
	cryptedPassword = Utils::convertToDataArray(packet.password, sizeof(packet.password), packet.password_size);

	debug("IMBC Login request for account %s\n", account.c_str());

	dbQuery = new DB_Account(this, account, getStream()->getRemoteIpStr(), useRsaAuth ? DB_Account::EM_AES : DB_Account::EM_None, cryptedPassword, aesKey);
}

void ClientSession::clientAuthResult(bool authOk, const std::string& account, uint32_t accountId, uint32_t age, uint16_t lastLoginServerIdx, uint32_t eventCode, uint32_t pcBang, uint32_t serverIdxOffset, bool block) {
	TS_AC_RESULT result;
	result.request_msg_id = TS_CA_ACCOUNT::packetID;

	dbQuery = nullptr;

	if(authOk == false) {
		result.result = TS_RESULT_NOT_EXIST;
		result.login_flag = 0;
	} else if(block == true) {
		result.result = TS_RESULT_ACCESS_DENIED;
		result.login_flag = 0;
	} else if(clientData != nullptr) { //already connected
		result.result = TS_RESULT_CLIENT_SIDE_ERROR;
		result.login_flag = 0;
		info("Client connection already authenticated with account %s\n", clientData->account.c_str());
	} else {
		ClientData* oldClient;
		clientData = ClientData::tryAddClient(this, account, accountId, age, eventCode, pcBang, getStream()->getRemoteIp(), &oldClient);
		if(clientData == nullptr) {
			result.result = TS_RESULT_ALREADY_EXIST;
			result.login_flag = 0;
			info("Client %s already connected\n", account.c_str());

			char ipStr[INET_ADDRSTRLEN];
			GameData* oldCientGameData = oldClient->getGameServer();

			uv_inet_ntop(AF_INET, &oldClient->ip, ipStr, sizeof(ipStr));

			if(!oldCientGameData) {
				LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_DUPLICATE_AUTH_LOGIN, accountId, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
										 account.c_str(), -1, getStream()->getRemoteIpStr(), -1, ipStr, -1, 0, 0);

				oldClient->getClientSession()->abortSession();
			} else {
				LogServerClient::sendLog(LogServerClient::LM_ACCOUNT_DUPLICATE_GAME_LOGIN, accountId, 0, 0, oldCientGameData->getServerIdx(), 0, 0, 0, 0, 0, 0, 0,
										 account.c_str(), -1, getStream()->getRemoteIpStr(), -1, ipStr, -1, 0, 0);

				if(oldClient->isConnectedToGame())
					oldCientGameData->kickClient(oldClient);
				else {
					ClientData::removeClient(oldClient);
				}
			}
		} else {
			result.result = 0;
			result.login_flag = LSF_EULA_ACCEPTED;
			this->lastLoginServerId = lastLoginServerIdx;
			this->serverIdxOffset = serverIdxOffset;
		}
	}

	sendPacket(result, version);
}

void ClientSession::onServerList(const TS_CA_SERVER_LIST& packet) {
	TS_AC_SERVER_LIST serverListPacket;

	int maxPublicServerBaseIdx = CONFIG_GET()->auth.client.maxPublicServerIdx;

	// Check if user authenticated
	if(clientData == nullptr) {
		abortSession();
		return;
	}

	const std::unordered_map<uint16_t, GameData*>& serverList = GameData::getServerList();
	std::unordered_map<uint16_t, GameData*>::const_iterator it, itEnd;

	unsigned int maxPlayers = CONFIG_GET()->auth.game.maxPlayers;

	serverListPacket.last_login_server_idx = lastLoginServerId;

	for(it = serverList.cbegin(), itEnd = serverList.cend(); it != itEnd; ++it) {
		GameData* serverInfo = it->second;

		//servers with their index higher than maxPublicServerBaseIdx + serverIdxOffset are hidden
		//serverIdxOffset is a per user value from the DB, default to 0
		//maxPublicServerBaseIdx is a config value, default to 30
		//So by default, servers with index > 30 are not shown in client's server list
		if(serverInfo->getServerIdx() > maxPublicServerBaseIdx + serverIdxOffset)
			continue;

		//Don't display not ready game servers (offline or not yet received all player list)
		if(!serverInfo->isReady())
			continue;

		TS_SERVER_INFO serverData;
		serverData.server_idx = serverInfo->getServerIdx();
		serverData.server_port = serverInfo->getServerPort();
		serverData.is_adult_server = serverInfo->getIsAdultServer();
		strcpy(serverData.server_ip, serverInfo->getServerIp().c_str());
		strcpy(serverData.server_name, serverInfo->getServerName().c_str());
		strcpy(serverData.server_screenshot_url, serverInfo->getServerScreenshotUrl().c_str());

		uint32_t userRatio = serverInfo->getPlayerCount() * 100 / maxPlayers;
		serverData.user_ratio = (userRatio > 100)? 100 : userRatio;

		serverListPacket.servers.push_back(serverData);
	}

	sendPacket(serverListPacket, version);
}

void ClientSession::onSelectServer(const TS_CA_SELECT_SERVER& packet) {
	const std::unordered_map<uint16_t, GameData*>& serverList = GameData::getServerList();

	if(clientData == nullptr) {
		abortSession();
		return;
	}

	if(serverList.find(packet.server_idx) != serverList.end()) {
		GameData* server = serverList.at(packet.server_idx);
		uint64_t oneTimePassword = ((uint64_t)rand())*rand()*rand()*rand();

		new DB_UpdateLastServerIdx(clientData->accountId, packet.server_idx);

		//clientData now managed by target GS
		clientData->switchClientToServer(server, oneTimePassword);
		clientData = nullptr;

		if(useRsaAuth) {
			TS_AC_SELECT_SERVER_RSA result;
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

			sendPacket(result, version);
			ok = true;

		cleanup:
			EVP_CIPHER_CTX_cleanup(&e_ctx);

			if(!ok)
				abortSession();
		} else {
			TS_AC_SELECT_SERVER result;

			result.result = 0;
			result.one_time_key = oneTimePassword;
			result.pending_time = 0;

			sendPacket(result, version);
		}
	} else {
		abortSession();
		warn("Attempt to connect to an invalid server idx: %d\n", packet.server_idx);
	}
}

} // namespace AuthServer
