#include "Authentication.h"
#include "Packets/AuthPackets.h"
#include "RappelzSocket.h"
#include "EventLoop.h"
#include "DesPasswordCipher.h"
#include "Account.h"
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>


DesPasswordCipher Authentication::desCipher("MERONG");

Authentication::Authentication(std::string host, AuthCipherMethod method, uint16_t port, const std::string& version) {
	this->authServer = new RappelzSocket(EventLoop::getLoop(), EncryptedSocket::Encrypted);
	this->gameServer = nullptr;

	this->authIp = host;
	this->authPort = port;
	this->cipherMethod = method;
	this->version = version;

	this->rsaCipher = 0;
	this->selectedServer = 0;
	this->inProgress = false;

	reserveCallbackCount(6);
	authServer->addPacketListener(TS_CC_EVENT::packetID, this, &onAuthServerConnectionEvent);
	authServer->addPacketListener(TS_AC_AES_KEY_IV::packetID, this, &onAuthPacketReceived);
	authServer->addPacketListener(TS_AC_RESULT::packetID, this, &onAuthPacketReceived);
	authServer->addPacketListener(TS_AC_RESULT_WITH_STRING::packetID, this, &onAuthPacketReceived);
	authServer->addPacketListener(TS_AC_SERVER_LIST::packetID, this, &onAuthPacketReceived);
	authServer->addPacketListener(TS_AC_SELECT_SERVER::packetID, this, &onAuthPacketReceived);

//	server->addPacketListener(Server::ST_Game, TS_CC_EVENT::packetID, this, &onGameServerConnectionEvent);
//	server->addPacketListener(Server::ST_Game, TS_CS_ACCOUNT_WITH_AUTH::packetID, this, &onGamePacketReceived);
}

Authentication::~Authentication() {
	if(inProgress)
		warn("Authentication object delete but authentication in progress !\n");
	if(authServer)
		delete authServer;
	if(gameServer)
		delete gameServer;
}

int Authentication::connect(Account* account, const std::string &password, Callback<CallbackOnAuthResult> callback) {
	if(account)
		this->username = account->getName();
	this->password = password;
	this->cipherMethod = cipherMethod;
	this->authResultCallback = callback;

	inProgress = true;

	authServer->connect(authIp, authPort);
	return 0;
}

void Authentication::abort(Callback<CallbackOnAuthClosed> callback) {
	authClosedCallback = callback;
	authServer->close();
	if(gameServer)
		gameServer->close();
	inProgress = false;
}

bool Authentication::retreiveServerList(Callback<CallbackOnServerList> callback) {
	TS_CA_SERVER_LIST getServerListMsg;

	serverListCallback = callback;

	TS_MESSAGE::initMessage<TS_CA_SERVER_LIST>(&getServerListMsg);
	authServer->sendPacket(&getServerListMsg);
	return true;
}

bool Authentication::selectServer(uint16_t serverId, Callback<CallbackOnGameResult> callback) {
	TS_CA_SELECT_SERVER selectServerMsg;

	selectedServer = -1;
	for(size_t i = 0; i < serverList.size(); i++) {
		if(serverList.at(i).id == serverId) {
			selectedServer = i;
			break;
		}
	}
	if(selectedServer == -1) {
		error("Can\'t select server, server list must be retrieved before\n");
		return false;
	}

	gameResultCallback = callback;

	TS_MESSAGE::initMessage<TS_CA_SELECT_SERVER>(&selectServerMsg);
	selectServerMsg.server_idx = serverId;
	authServer->sendPacket(&selectServerMsg);

	return true;
}

void Authentication::onAuthServerConnectionEvent(IListener* instance, RappelzSocket*, const TS_MESSAGE* packetData) {
	Authentication* thisAccount = static_cast<Authentication*>(instance);

	if(packetData->id == TS_CC_EVENT::packetID) {
		const TS_CC_EVENT* eventMsg = reinterpret_cast<const TS_CC_EVENT*>(packetData);

		if(eventMsg->event == TS_CC_EVENT::CE_ServerConnected)
			thisAccount->onPacketAuthConnected();
		else if(eventMsg->event == TS_CC_EVENT::CE_ServerDisconnected)
			thisAccount->onPacketAuthClosed();
		else
			thisAccount->onPacketAuthUnreachable();
	}
}

void Authentication::onGameServerConnectionEvent(IListener* instance, RappelzSocket*, const TS_MESSAGE* packetData) {
	Authentication* thisAccount = static_cast<Authentication*>(instance);

	if(packetData->id == TS_CC_EVENT::packetID) {
		const TS_CC_EVENT* eventMsg = reinterpret_cast<const TS_CC_EVENT*>(packetData);

		if(eventMsg->event == TS_CC_EVENT::CE_ServerConnected)
			thisAccount->onPacketGameConnected();
		else
			thisAccount->onPacketGameUnreachable();
	}
}

void Authentication::onAuthPacketReceived(IListener* instance, RappelzSocket*, const TS_MESSAGE* packetData) {
	Authentication* thisAccount = static_cast<Authentication*>(instance);

	switch(packetData->id) {
		case TS_AC_AES_KEY_IV::packetID:
			thisAccount->onPacketAuthPasswordKey(reinterpret_cast<const TS_AC_AES_KEY_IV*>(packetData));
			break;

		case TS_AC_RESULT::packetID:
		{
			const TS_AC_RESULT* resultMsg = reinterpret_cast<const TS_AC_RESULT*>(packetData);
			if(resultMsg->request_msg_id == TS_CA_ACCOUNT::packetID) {
				CALLBACK_CALL(thisAccount->authResultCallback, thisAccount, (TS_ResultCode)resultMsg->result, std::string());
			}
			break;
		}

		case TS_AC_RESULT_WITH_STRING::packetID:
		{
			const TS_AC_RESULT_WITH_STRING* resultMsg = reinterpret_cast<const TS_AC_RESULT_WITH_STRING*>(packetData);
			if(resultMsg->request_msg_id == TS_CA_ACCOUNT::packetID) {
				CALLBACK_CALL(thisAccount->authResultCallback, thisAccount, (TS_ResultCode)resultMsg->result, std::string(resultMsg->string, resultMsg->strSize));
			}
			break;
		}

		case TS_AC_SERVER_LIST::packetID:
			thisAccount->onPacketServerList(reinterpret_cast<const TS_AC_SERVER_LIST*>(packetData));
			break;

		case TS_AC_SELECT_SERVER::packetID:
			thisAccount->onPacketSelectServerResult(reinterpret_cast<const TS_AC_SELECT_SERVER*>(packetData));
			break;
	}
}

void Authentication::onGamePacketReceived(IListener* instance, RappelzSocket*, const TS_MESSAGE* packetData) {
	Authentication* thisAccount = static_cast<Authentication*>(instance);

	switch(packetData->id) {
		case TS_SC_RESULT::packetID:
		{
			const TS_SC_RESULT* resultMsg = reinterpret_cast<const TS_SC_RESULT*>(packetData);
			if(resultMsg->request_msg_id == TS_CS_ACCOUNT_WITH_AUTH::packetID)
				thisAccount->onPacketGameAuthResult(resultMsg);
			break;
		}
	}
}

////////////////////////////////////////////////////////

void Authentication::onPacketAuthConnected() {
	TS_CA_VERSION versionMsg;

	if(!authResultCallback.callback)
		return;

	TS_MESSAGE::initMessage<TS_CA_VERSION>(&versionMsg);
#ifndef NDEBUG
	memset(versionMsg.szVersion, 0, sizeof(versionMsg.szVersion));
#endif
	strcpy(versionMsg.szVersion, version.c_str());
	authServer->sendPacket(&versionMsg);

	if(this->cipherMethod == ACM_DES) {
		TS_CA_ACCOUNT accountMsg;

		TS_MESSAGE::initMessage<TS_CA_ACCOUNT>(&accountMsg);

#ifndef NDEBUG
		memset(accountMsg.account, 0, 61);
		memset(accountMsg.password, 0, 61);
#endif
		strcpy(accountMsg.account, username.c_str());

		static char cachedPassword[61] = {0};
		static std::string cachedPasswordStr;

		if(cachedPasswordStr != password) {
			strcpy(cachedPassword, password.c_str());
			desCipher.encrypt(cachedPassword, 61);
			cachedPasswordStr = password;
		}

		strcpy(accountMsg.password, cachedPassword);

		authServer->sendPacket(&accountMsg);
	} else if(this->cipherMethod == ACM_RSA_AES) {
		TS_CA_RSA_PUBLIC_KEY *keyMsg;
		int public_key_size;

		rsaCipher = RSA_generate_key(1024, 65537, NULL, NULL);

		BIO * b = BIO_new(BIO_s_mem());
		PEM_write_bio_RSA_PUBKEY(b, (RSA*)rsaCipher);

		public_key_size = BIO_get_mem_data(b, NULL);
		keyMsg = TS_MESSAGE_WNA::create<TS_CA_RSA_PUBLIC_KEY, unsigned char>(public_key_size);

		keyMsg->key_size = public_key_size;

		BIO_read(b, keyMsg->key, public_key_size);
		BIO_free(b);

		authServer->sendPacket(keyMsg);
		TS_MESSAGE_WNA::destroy(keyMsg);
	}
}

void Authentication::onPacketAuthClosed() {
	CALLBACK_CALL(authClosedCallback, this);

	inProgress = false;
}

void Authentication::onPacketAuthUnreachable() {
	CALLBACK_CALL(authResultCallback, this, TS_RESULT_UNKNOWN, "Unable to connect to authentication server");
	inProgress = false;
}

void Authentication::onPacketAuthPasswordKey(const TS_AC_AES_KEY_IV* packet) {
	TS_CA_ACCOUNT_RSA accountMsg;
	unsigned char decrypted_data[256];
	int data_size;
	bool ok = false;

	TS_MESSAGE::initMessage<TS_CA_ACCOUNT_RSA>(&accountMsg);
	memset(accountMsg.account, 0, sizeof(accountMsg.account));

	EVP_CIPHER_CTX e_ctx;
	const unsigned char *key_data = decrypted_data;
	const unsigned char *iv_data = decrypted_data + 16;
	int len = password.size();
	int p_len = len, f_len = 0;

	data_size = RSA_private_decrypt(packet->data_size, (unsigned char*)packet->rsa_encrypted_data, decrypted_data, (RSA*)rsaCipher, RSA_PKCS1_PADDING);
	RSA_free((RSA*)rsaCipher);
	rsaCipher = 0;
	if(data_size != 32) {
		warn("onPacketAuthPasswordKey: invalid decrypted data size: %d\n", data_size);
		authServer->close();
		return;
	}

	memcpy(aes_key_iv, decrypted_data, 32);
	memset(accountMsg.password, 0, sizeof(accountMsg.password));

	EVP_CIPHER_CTX_init(&e_ctx);

	if(!EVP_EncryptInit_ex(&e_ctx, EVP_aes_128_cbc(), NULL, key_data, iv_data))
		goto end;
	if(!EVP_EncryptInit_ex(&e_ctx, NULL, NULL, NULL, NULL))
		goto end;
	if(!EVP_EncryptUpdate(&e_ctx, accountMsg.password, &p_len, (const unsigned char*)password.c_str(), len))
		goto end;
	if(!EVP_EncryptFinal_ex(&e_ctx, accountMsg.password+p_len, &f_len))
		goto end;

	ok = true;

end:
	EVP_CIPHER_CTX_cleanup(&e_ctx);

	if(ok == false) {
		warn("onPacketAuthPasswordKey: could not encrypt password !\n");
		authServer->close();
		return;
	}

	//password.fill(0);
	password.clear();

	strcpy(accountMsg.account, username.c_str());
	accountMsg.aes_block_size = p_len + f_len;
	accountMsg.dummy[0] = accountMsg.dummy[1] = accountMsg.dummy[2] = 0;
	accountMsg.unknown_00000100 = 0x00000100;
	authServer->sendPacket(&accountMsg);
}

void Authentication::onPacketServerList(const TS_AC_SERVER_LIST* packet) {
	std::vector<ServerInfo> serverList;
	ServerInfo currentServerInfo;
	ServerConnectionInfo serverConnectionInfo;
	const TS_AC_SERVER_LIST::TS_SERVER_INFO* packetServerList = packet->servers;

	serverList.reserve(packet->count);
	selectedServer = 0;
	this->serverList.clear();

	for(int i=0; i < packet->count; ++i) {
		currentServerInfo.serverId = packetServerList[i].server_idx;
		currentServerInfo.serverIp = std::string(packetServerList[i].server_ip);
		currentServerInfo.serverPort = packetServerList[i].server_port;
		currentServerInfo.serverName = std::string(packetServerList[i].server_name);
		currentServerInfo.serverScreenshotUrl = std::string(packetServerList[i].server_screenshot_url);
		currentServerInfo.userRatio = packetServerList[i].user_ratio;
		serverList.push_back(currentServerInfo);

		//Keep in memory server's ip & port for the server move
		serverConnectionInfo.id = currentServerInfo.serverId;
		serverConnectionInfo.ip = currentServerInfo.serverIp;
		serverConnectionInfo.port = currentServerInfo.serverPort;
		this->serverList.push_back(serverConnectionInfo);

		if(packetServerList[i].server_idx == packet->last_login_server_idx)
			selectedServer = i;
	}

	CALLBACK_CALL(serverListCallback, this, &serverList, packet->last_login_server_idx);
}

void Authentication::onPacketSelectServerResult(const TS_AC_SELECT_SERVER* packet) {
	//int pendingTimeBeforeServerMove;

	if(this->cipherMethod == ACM_DES) {
		oneTimePassword = packet->one_time_key;
		//pendingTimeBeforeServerMove = packet->v1.pending_time;
	} else if(this->cipherMethod == ACM_RSA_AES) {
		const TS_AC_SELECT_SERVER_RSA* packetv2 = reinterpret_cast<const TS_AC_SELECT_SERVER_RSA*>(packet);
		EVP_CIPHER_CTX e_ctx;
		const unsigned char *key_data = aes_key_iv;
		const unsigned char *iv_data = aes_key_iv + 16;
		int len = 16;
		int p_len = 0, f_len = 0;
		bool ok = false;
		uint64_t decryptedData[2];

		EVP_CIPHER_CTX_init(&e_ctx);
		if(!EVP_DecryptInit_ex(&e_ctx, EVP_aes_128_cbc(), NULL, key_data, iv_data))
			goto end;
		if(!EVP_DecryptInit_ex(&e_ctx, NULL, NULL, NULL, NULL))
			goto end;
		if(!EVP_DecryptUpdate(&e_ctx, (unsigned char*)decryptedData, &p_len, packetv2->encrypted_data, len))
			goto end;
		if(!EVP_DecryptFinal_ex(&e_ctx, ((unsigned char*)decryptedData) + p_len, &f_len))
			goto end;
		ok = true;
end:
		EVP_CIPHER_CTX_cleanup(&e_ctx);

		if(ok == false) {
			warn("onPacketSelectServerResult: Could not decrypt TS_AC_SELECT_SERVER\n");
			authServer->close();
			return;
		}

		oneTimePassword = decryptedData[0];
		//pendingTimeBeforeServerMove = packetv2->pending_time;
	}

	ServerConnectionInfo selectedServerInfo = serverList.at(selectedServer);
	gameServer = new RappelzSocket(EventLoop::getLoop(), EncryptedSocket::Encrypted);
	gameServer->addPacketListener(TS_CC_EVENT::packetID, this, &onGameServerConnectionEvent);
	gameServer->addPacketListener(TS_CS_ACCOUNT_WITH_AUTH::packetID, this, &onGamePacketReceived);
	gameServer->connect(selectedServerInfo.ip, selectedServerInfo.port);
	authServer->close();
}

void Authentication::onPacketGameConnected() {
	TS_CS_VERSION versionMsg;
	TS_CS_ACCOUNT_WITH_AUTH loginInGameServerMsg;

	//continue server move as we are connected now to game server
	TS_MESSAGE::initMessage<TS_CS_VERSION>(&versionMsg);
	strcpy(versionMsg.szVersion, version.c_str());
	gameServer->sendPacket(&versionMsg);

	TS_MESSAGE::initMessage<TS_CS_ACCOUNT_WITH_AUTH>(&loginInGameServerMsg);
	strcpy(loginInGameServerMsg.account, username.c_str());
	loginInGameServerMsg.one_time_key = oneTimePassword;
	gameServer->sendPacket(&loginInGameServerMsg);
}

void Authentication::onPacketGameUnreachable() {
	CALLBACK_CALL(gameResultCallback, this, TS_RESULT_NOT_EXIST, nullptr);

	inProgress = false;
}

void Authentication::onPacketGameAuthResult(const TS_SC_RESULT* packet) {
	gameServer->removePacketListener(TS_CC_EVENT::packetID, this);
	gameServer->removePacketListener(TS_CS_ACCOUNT_WITH_AUTH::packetID, this);

	if(packet->result == 0)
		CALLBACK_CALL(gameResultCallback, this, TS_RESULT_SUCCESS, gameServer);
	else
		CALLBACK_CALL(gameResultCallback, this, TS_RESULT_MISC, nullptr);

	gameResultCallback = nullptr;

	inProgress = false;
}
