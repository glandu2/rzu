#include "ClientInfo.h"
#include "ServerInfo.h"
#include "Network/RappelzSocket.h"
#include <string.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "DesPasswordCipher.h"
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "Packets/TS_AC_RESULT.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_AC_AES_KEY_IV.h"
#include "Packets/TS_AC_SELECT_SERVER.h"
#include "Packets/TS_AC_SERVER_LIST.h"

Socket* ClientInfo::serverSocket = new Socket;
std::unordered_map<std::string, ClientData*> ClientInfo::pendingClients;

ClientInfo::ClientInfo(RappelzSocket* socket) {
	this->socket = socket;
	this->useRsaAuth = false;

	addInstance(socket->addEventListener(this, &onStateChanged));
	addInstance(socket->addPacketListener(TS_CA_VERSION::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_CA_RSA_PUBLIC_KEY::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_CA_ACCOUNT::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_CA_SERVER_LIST::packetID, this, &onDataReceived));
	addInstance(socket->addPacketListener(TS_CA_SELECT_SERVER::packetID, this, &onDataReceived));
}

ClientInfo::~ClientInfo() {
	invalidateCallbacks();
	if(clientData) {
		pendingClients.erase(clientData->account);
		delete clientData;
	}

	socket->deleteLater();
}

void ClientInfo::startServer() {
	srand(time(NULL));
	serverSocket->addConnectionListener(nullptr, &onNewConnection);
	serverSocket->listen("0.0.0.0", 4500);
}

ClientData* ClientInfo::popPendingClient(const std::string& accountName) {
	std::unordered_map<std::string, ClientData*>::const_iterator it;

	it = pendingClients.find(accountName);
	if(it != pendingClients.cend()) {
		ClientData* clientData = it->second;
		pendingClients.erase(it);
		return clientData;
	} else
		return nullptr;
}

void ClientInfo::onNewConnection(void* instance, Socket* serverSocket) {
	static RappelzSocket *newSocket = new RappelzSocket(true);
	static ClientInfo* clientInfo = new ClientInfo(newSocket);

	do {

		if(!serverSocket->accept(newSocket))
			break;

		printf("new client connection\n");
		newSocket = new RappelzSocket(true);
		clientInfo = new ClientInfo(newSocket);
	} while(1);
}

void ClientInfo::onStateChanged(void* instance, Socket* clientSocket, Socket::State oldState, Socket::State newState) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);
	
	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}

void ClientInfo::onDataReceived(void* instance, RappelzSocket*, const TS_MESSAGE* packet) {
	ClientInfo* thisInstance = static_cast<ClientInfo*>(instance);

	switch(packet->id) {
		case TS_CA_VERSION::packetID:
			thisInstance->onVersion(static_cast<const TS_CA_VERSION*>(packet));
			break;

		case TS_CA_RSA_PUBLIC_KEY::packetID:
			thisInstance->onRsaKey(static_cast<const TS_CA_RSA_PUBLIC_KEY*>(packet));
			break;

		case TS_CA_ACCOUNT::packetID:
			thisInstance->onAccount(static_cast<const TS_CA_ACCOUNT*>(packet));
			break;

		case TS_CA_SERVER_LIST::packetID:
			thisInstance->onServerList(static_cast<const TS_CA_SERVER_LIST*>(packet));
			break;

		case TS_CA_SELECT_SERVER::packetID:
			thisInstance->onSelectServer(static_cast<const TS_CA_SELECT_SERVER*>(packet));
			break;
	}
}

void ClientInfo::onVersion(const TS_CA_VERSION* packet) {
	if(!strcmp(packet->szVersion, "TEST")) {
		uint32_t totalUserCount = 0;
		const std::vector<ServerInfo*>& serverList = ServerInfo::getServerList();
		TS_SC_RESULT result;
		TS_MESSAGE::initMessage<TS_SC_RESULT>(&result);

		for(size_t i = 0; i < serverList.size(); i++) {
			ServerInfo* serverInfo = serverList.at(i);
			if(serverInfo)
				totalUserCount += serverInfo->getUserCount();
		}

		result.value = totalUserCount ^ 0xADADADAD;
		result.result = 0;
		result.request_msg_id = packet->id;
		socket->sendPacket(&result);
	}
}

void ClientInfo::onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet) {
	TS_AC_AES_KEY_IV* aesKeyMessage = nullptr;
	RSA* rsaCipher;
	BIO* bio;
	int blockSize;

	for(int i = 0; i < 32; i++)
		aesKey[i] = rand() & 0xFF;

	bio = BIO_new_mem_buf((void*)packet->key, packet->key_size);
	rsaCipher = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
	if(rsaCipher == nullptr) {
		fprintf(stderr, "ClientInfo: RSA: invalid certificate\n");
		socket->abort();
		goto cleanup;
	}

	aesKeyMessage = TS_MESSAGE_WNA::create<TS_AC_AES_KEY_IV, unsigned char>(RSA_size(rsaCipher));

	blockSize = RSA_public_encrypt(32, aesKey, aesKeyMessage->rsa_encrypted_data, rsaCipher, RSA_PKCS1_PADDING);
	if(blockSize < 0) {
		const char* errorString = ERR_error_string(ERR_get_error(), nullptr);
		fprintf(stderr, "ClientInfo: RSA encrypt error: %s\n", errorString);
		socket->abort();
		goto cleanup;
	}

	aesKeyMessage->data_size = blockSize;

	useRsaAuth = true;
	socket->sendPacket(aesKeyMessage);

cleanup:
	if(aesKeyMessage)
		TS_MESSAGE_WNA::destroy(aesKeyMessage);
	BIO_free(bio);
	RSA_free(rsaCipher);
}

void ClientInfo::onAccount(const TS_CA_ACCOUNT* packet) {
	unsigned char password[64];  //size = at most rsa encrypted size

	clientData = new ClientData;

	if(useRsaAuth) {
		const TS_CA_ACCOUNT_V2* accountv2 = reinterpret_cast<const TS_CA_ACCOUNT_V2*>(packet);
		EVP_CIPHER_CTX d_ctx;
		int bytesWritten, totalLength = 0;
		unsigned int bytesRead;

		printf("Client login using AES %s\n", accountv2->account);

		clientData->account = std::string(accountv2->account, 60);

		EVP_CIPHER_CTX_init(&d_ctx);
		if(EVP_DecryptInit_ex(&d_ctx, EVP_aes_128_cbc(), NULL, aesKey, aesKey + 16) < 0)
			goto cleanup_aes;
		if(EVP_DecryptInit_ex(&d_ctx, NULL, NULL, NULL, NULL) < 0)
			goto cleanup_aes;

		for(bytesRead = 0; bytesRead + 15 < accountv2->aes_block_size; bytesRead += 16) {
			if(EVP_DecryptUpdate(&d_ctx, password + totalLength, &bytesWritten, accountv2->password + bytesRead, 16) < 0)
				goto cleanup_aes;
			totalLength += bytesWritten;
		}

		if(EVP_DecryptFinal_ex(&d_ctx, password + totalLength, &bytesWritten) < 0)
			goto cleanup_aes;

		totalLength += bytesWritten;

		if(totalLength >= 64)
			goto cleanup_aes;

		password[totalLength] = 0;

	cleanup_aes:
		EVP_CIPHER_CTX_cleanup(&d_ctx);
	} else {
		printf("Client login using DES %s\n", packet->account);

		clientData->account = std::string(packet->account, 60);

		strncpy((char*)password, packet->password, 61);
		DesPasswordCipher("MERONG").decrypt(password, 61);
	}

	printf("Login request for account %s with password %s\n", clientData->account.c_str(), password);
	memset(password, 0, 64);

	clientData->accountId = 0;
	clientData->age = 0;
	clientData->lastLoginServerId = 0;
	clientData->eventCode = 0;

	TS_AC_RESULT result;
	TS_MESSAGE::initMessage<TS_AC_RESULT>(&result);
	result.request_msg_id = TS_CA_ACCOUNT::packetID;
	result.result = 0;
	result.login_flag = TS_AC_RESULT::LSF_EULA_ACCEPTED;
	socket->sendPacket(&result);
}

void ClientInfo::onServerList(const TS_CA_SERVER_LIST* packet) {
	TS_AC_SERVER_LIST* serverListPacket;
	int i, j, count;

	// Check if user authenticated
	if(clientData == nullptr) {
		socket->abort();
		return;
	}

	const std::vector<ServerInfo*>& serverList = ServerInfo::getServerList();

	for(i = count = 0; i < serverList.size(); i++)
		if(serverList.at(i) != nullptr)
			count++;

	serverListPacket = TS_MESSAGE_WNA::create<TS_AC_SERVER_LIST, TS_AC_SERVER_LIST::TS_SERVER_INFO>(count);

	serverListPacket->count = count;
	serverListPacket->last_login_server_idx = 0;

	for(i = j = 0; i < serverList.size() && j < serverListPacket->count; i++) {
		ServerInfo* serverInfo = serverList.at(i);

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

	socket->sendPacket(serverListPacket);
	TS_MESSAGE_WNA::destroy(serverListPacket);
}

void ClientInfo::onSelectServer(const TS_CA_SELECT_SERVER* packet) {
	const std::vector<ServerInfo*>& serverList = ServerInfo::getServerList();

	if(clientData == nullptr) {
		socket->abort();
		return;
	}

	if(packet->server_idx < serverList.size()) {
		ServerInfo* server = serverList.at(packet->server_idx);

		clientData->oneTimePassword = (uint64_t)rand()*rand()*rand()*rand();

		popPendingClient(clientData->account);
		server->addPendingClient(clientData);

		if(useRsaAuth) {
			TS_AC_SELECT_SERVER_V2 result;
			TS_MESSAGE::initMessage<TS_AC_SELECT_SERVER_V2>(&result);
			result.result = 0;
			result.encrypted_data_size = 16;
			result.pending_time = 10;
			result.unknown = 0;
			result.unknown2 = 0;

			EVP_CIPHER_CTX e_ctx;
			int bytesWritten;

			EVP_CIPHER_CTX_init(&e_ctx);
			EVP_EncryptInit_ex(&e_ctx, EVP_aes_128_cbc(), NULL, aesKey, aesKey + 16);
			EVP_EncryptInit_ex(&e_ctx, NULL, NULL, NULL, NULL);
			EVP_EncryptUpdate(&e_ctx, result.encrypted_data, &bytesWritten, (const unsigned char*)&clientData->oneTimePassword, sizeof(uint64_t));
			EVP_EncryptFinal_ex(&e_ctx, result.encrypted_data + bytesWritten, &bytesWritten);

			EVP_CIPHER_CTX_cleanup(&e_ctx);

			socket->sendPacket(&result);
		} else {
			TS_AC_SELECT_SERVER result;
			TS_MESSAGE::initMessage<TS_AC_SELECT_SERVER>(&result);

			result.result = 0;
			result.one_time_key = clientData->oneTimePassword;
			result.pending_time = 10;

			socket->sendPacket(&result);
		}

		clientData = nullptr;
	} else {
		socket->abort();
	}
}
