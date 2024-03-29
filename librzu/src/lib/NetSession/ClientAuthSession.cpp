#include "ClientAuthSession.h"
#include "Cipher/AesPasswordCipher.h"
#include "Cipher/DesPasswordCipher.h"
#include "Cipher/RsaCipher.h"
#include "ClientGameSession.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Packet/PacketEpics.h"
#include "Packet/PacketVersionUtils.h"
#include "Stream/Curl.h"

#include "AuthClient/TS_AC_ACCOUNT_NAME.h"
#include "AuthClient/TS_AC_AES_KEY_IV.h"
#include "AuthClient/TS_AC_RESULT.h"
#include "AuthClient/TS_AC_RESULT_WITH_STRING.h"
#include "AuthClient/TS_AC_SELECT_SERVER.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"

#include "AuthClient/TS_CA_ACCOUNT.h"
#include "AuthClient/TS_CA_RSA_PUBLIC_KEY.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "AuthClient/TS_CA_SERVER_LIST.h"
#include "AuthClient/TS_CA_VERSION.h"

DesPasswordCipher ClientAuthSession::desCipher("MERONG");

ClientAuthSession::ClientAuthSession(ClientGameSession* gameSession, int packetVersion)
    : EncryptedSession<PacketSession>(SessionType::AuthClient, SessionPacketOrigin::Client, packetVersion),
      gameSession(gameSession) {
	if(gameSession)
		gameSession->setAuthSession(this);
}

ClientAuthSession::~ClientAuthSession() {}

bool ClientAuthSession::connect(const std::string& ip,
                                uint16_t port,
                                const std::string& account,
                                const std::string& password) {
	this->username = account;
	this->password = password;
	this->authVersion = PacketVersionUtils::getAuthVersionString(packetVersion);
	this->gameVersion = PacketVersionUtils::getGameVersionString(packetVersion);
	this->cipherMethod = packetVersion >= EPIC_8_1_1_RSA ? ClientAuthSession::ACM_RSA_AES : ClientAuthSession::ACM_DES;
	//	this->rsaCipher = 0;
	this->selectedServer = 0;
	this->normalDisconnect = false;

	return connect(ip.c_str(), port);
}

void ClientAuthSession::close() {
	closeSession();
	if(gameSession)
		gameSession->close();
}

void ClientAuthSession::retreiveServerList() {
	TS_CA_SERVER_LIST getServerListMsg{};

	sendPacket(getServerListMsg);
}

bool ClientAuthSession::selectServer(uint16_t serverId) {
	TS_CA_SELECT_SERVER selectServerMsg;

	selectedServer = -1;
	for(size_t i = 0; i < serverList.size(); i++) {
		if(serverList.at(i).id == serverId) {
			selectedServer = (int) i;
			break;
		}
	}
	if(selectedServer == -1) {
		log(LL_Error, "Can\'t select server, server id is invalid or server list must be retrieved before\n");
		return false;
	}

	selectServerMsg.server_idx = serverId;
	sendPacket(selectServerMsg);

	return true;
}

EventChain<SocketSession> ClientAuthSession::onDisconnected(bool causedByRemote) {
	if(normalDisconnect == false || gameSession == nullptr) {
		onAuthDisconnected(causedByRemote);
	} else {
		normalDisconnect = false;
	}

	return PacketSession::onDisconnected(causedByRemote);
}

EventChain<PacketSession> ClientAuthSession::onPacketReceived(const TS_MESSAGE* packet) {
	packet_type_id_t packetType = PacketMetadata::convertPacketIdToTypeId(
	    packet->id, SessionType::AuthClient, SessionPacketOrigin::Server, packetVersion);
	switch(packetType) {
		case TS_AC_AES_KEY_IV::packetID:
			packet->process(this, &ClientAuthSession::onPacketAuthPasswordKey, packetVersion);
			break;

		case TS_AC_ACCOUNT_NAME::packetID:
			packet->process(this, &ClientAuthSession::onPacketAccountName, packetVersion);
			break;

		case TS_AC_RESULT::packetID:
			packet->process(this, &ClientAuthSession::onPacketAuthResult, packetVersion);
			break;

		case TS_AC_RESULT_WITH_STRING::packetID:
			packet->process(this, &ClientAuthSession::onPacketAuthStringResult, packetVersion);
			break;

		case TS_AC_SERVER_LIST::packetID:
			packet->process(this, &ClientAuthSession::onPacketServerList, packetVersion);
			break;

		case TS_AC_SELECT_SERVER::packetID:
			packet->process(this, &ClientAuthSession::onPacketSelectServerResult, packetVersion);
			break;
	}

	return PacketSession::onPacketReceived(packet);
}

////////////////////////////////////////////////////////

RsaCipher ClientAuthSession::rsaCipher;
EventChain<SocketSession> ClientAuthSession::onConnected() {
	TS_CA_VERSION versionMsg;

	versionMsg.szVersion = authVersion;
	sendPacket(versionMsg);

	if(this->cipherMethod == ACM_DES) {
		TS_CA_ACCOUNT accountMsg;

		log(LL_Debug,
		    "Connecting to auth using DES with account %s and password %s\n",
		    username.c_str(),
		    password.c_str());

		static unsigned char cachedPassword[sizeof(accountMsg.passwordDes.password)] = {0};
		static std::string cachedPasswordStr;

		if(cachedPasswordStr != password) {
			strcpy(reinterpret_cast<char*>(cachedPassword), password.c_str());
			desCipher.encrypt(cachedPassword, packetVersion >= EPIC_5_2 ? 61 : 32);
			cachedPasswordStr = password;
		}

		accountMsg.account = username;
		memcpy(accountMsg.passwordDes.password, cachedPassword, sizeof(accountMsg.passwordDes.password));

		sendPacket(accountMsg);
	} else if(this->cipherMethod == ACM_RSA_AES) {
		log(LL_Debug, "Connecting to auth using AES with account %s\n", username.c_str());

		if(!rsaCipher.isInitialized()) {
			if(!rsaCipher.generateKey()) {
				log(LL_Error, "Failed to generate RSA key\n");
			}
		}

		if(rsaCipher.isInitialized()) {
			TS_CA_RSA_PUBLIC_KEY keyMsg;

			rsaCipher.getPemPublicKey(keyMsg.key);

			sendPacket(keyMsg);
		} else {
			log(LL_Error, "No RSA key to send, aborting connection\n");
			abortSession();
		}
	}

	return PacketSession::onConnected();
}

void ClientAuthSession::onPacketAuthPasswordKey(const TS_AC_AES_KEY_IV* packet) {
	if(!rsaCipher.privateDecrypt(packet->data.data(), packet->data.size(), aesKey) || aesKey.size() != 32) {
		log(LL_Warning, "onPacketAuthPasswordKey: invalid decrypted data size: %d\n", (int) aesKey.size());
		closeSession();
		return;
	}

	if(username != "bora") {
		TS_CA_ACCOUNT accountMsg;
		std::vector<uint8_t> encryptedPassword;
		AesPasswordCipher aesCipher;
		aesCipher.init(aesKey.data());

		if(!aesCipher.encrypt((const uint8_t*) password.c_str(), password.size(), encryptedPassword)) {
			log(LL_Warning, "onPacketAuthPasswordKey: could not encrypt password !\n");
			closeSession();
			return;
		}

		accountMsg.passwordAes.password_size = (int) encryptedPassword.size();

		if(packetVersion >= EPIC_9_6_7) {
			// The client can use a maximum of 516 bytes for the password
			if(encryptedPassword.size() > 516) {
				log(LL_Warning, "onPacketAuthPasswordKey: encrypted password too large !\n");
				closeSession();
				return;
			}
			accountMsg.passwordAes.password_dyn = std::move(encryptedPassword);
		} else {
			if(encryptedPassword.size() > sizeof(accountMsg.passwordAes.password)) {
				log(LL_Warning, "onPacketAuthPasswordKey: encrypted password too large !\n");
				closeSession();
				return;
			}
			memcpy(accountMsg.passwordAes.password, encryptedPassword.data(), encryptedPassword.size());
		}

		accountMsg.account = username;
		sendPacket(accountMsg);
	} else {
		if(!curl)
			curl.reset(new Curl);
		char cookies[128];
		snprintf(cookies, sizeof(cookies), "SESSION=%s;", password.c_str());

		log(LL_Debug, "Using bora login with session %s\n", password.c_str());

		CURLM* curlHandle = curl_easy_init();
		curl_easy_setopt(
		    curlHandle, CURLOPT_URL, GlobalCoreConfig::get()->client.boraCodeGeneratorCommand.get().c_str());
		curl_easy_setopt(curlHandle, CURLOPT_NOBODY, 1);
		curl_easy_setopt(curlHandle, CURLOPT_COOKIE, cookies);
		curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1);

		curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 0);
		curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 0);

		curl->addHandle(
		    curlHandle,
		    [](CURLM* curlHandle, CURLcode result, void* arg) {
			    ClientAuthSession* thisInstance = (ClientAuthSession*) arg;
			    const char* location = nullptr;
			    long httpStatus = -1;

			    if(result != CURLE_OK) {
				    thisInstance->log(LL_Error, "Failed to query URL: %d\n", result);
				    thisInstance->closeSession();
			    } else if(curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &httpStatus) != CURLE_OK ||
			              httpStatus >= 400) {
				    thisInstance->log(LL_Error, "OAuth Server replied HTTP error code %ld\n", httpStatus);
				    thisInstance->closeSession();
			    } else if(curl_easy_getinfo(curlHandle, CURLINFO_REDIRECT_URL, &location) != CURLE_OK ||
			              location == nullptr) {
				    thisInstance->log(
				        LL_Error, "URL has no location (not a redirect?), http status is %ld\n", httpStatus);
				    thisInstance->closeSession();
			    } else {
				    thisInstance->log(LL_Info, "Received location: %s with http status %ld\n", location, httpStatus);
				    const char* lastEqualPos = strstr(location, "code=");

				    if(lastEqualPos) {
					    TS_CA_ACCOUNT accountMsg;

					    accountMsg.passwordAes.password_size = 0;

					    if(thisInstance->packetVersion >= EPIC_9_6_7) {
						    const uint8_t* data_to_copy = reinterpret_cast<const uint8_t*>(lastEqualPos + 5);
						    size_t size_to_copy = strlen(lastEqualPos + 5) + 1;
						    accountMsg.passwordAes.password_dyn.assign(data_to_copy, data_to_copy + size_to_copy);
					    } else {
						    memset(accountMsg.passwordAes.password, 0, sizeof(accountMsg.passwordAes.password));
						    strcpy((char*) accountMsg.passwordAes.password, lastEqualPos + 5);
					    }
					    accountMsg.account = "bora";
					    thisInstance->sendPacket(accountMsg);
				    } else {
					    thisInstance->log(LL_Error, "Cannot find '=' in redirected url %s\n", location);
					    thisInstance->closeSession();
				    }
			    }
		    },
		    this);
	}

	// password.fill(0);
	password.clear();
}

void ClientAuthSession::onPacketServerList(const TS_AC_SERVER_LIST* packet) {
	std::vector<ServerInfo> serverList;
	ServerInfo currentServerInfo;
	ServerConnectionInfo serverConnectionInfo;
	auto& packetServerList = packet->servers;

	this->serverList.clear();
	serverList.reserve(packetServerList.size());
	this->serverList.reserve(packetServerList.size());

	selectedServer = 0;

	for(size_t i = 0; i < packetServerList.size(); ++i) {
		currentServerInfo.serverId = packetServerList[i].server_idx;
		currentServerInfo.serverIp = packetServerList[i].server_ip;
		currentServerInfo.serverPort = packetServerList[i].server_port;
		currentServerInfo.serverName = packetServerList[i].server_name;
		currentServerInfo.serverScreenshotUrl = packetServerList[i].server_screenshot_url;
		currentServerInfo.userRatio = packetServerList[i].user_ratio;
		serverList.push_back(currentServerInfo);

		// Keep in memory server's ip & port for the server move
		serverConnectionInfo.id = currentServerInfo.serverId;
		serverConnectionInfo.ip = currentServerInfo.serverIp;
		serverConnectionInfo.port = currentServerInfo.serverPort;
		this->serverList.push_back(serverConnectionInfo);

		if(packetServerList[i].server_idx == packet->last_login_server_idx)
			selectedServer = (int) i;
	}

	onServerList(serverList, packet->last_login_server_idx);
}

void ClientAuthSession::onPacketSelectServerResult(const TS_AC_SELECT_SERVER* packet) {
	// int pendingTimeBeforeServerMove;

	if(this->cipherMethod == ACM_DES) {
		oneTimePassword = packet->one_time_key;
		// pendingTimeBeforeServerMove = packet->v1.pending_time;
	} else if(this->cipherMethod == ACM_RSA_AES) {
		std::vector<uint8_t> decryptedData;
		size_t encryptedSize = packet->encrypted_data_size;

		AesPasswordCipher aesCipher;
		aesCipher.init(aesKey.data());

		if(encryptedSize > sizeof(packet->encrypted_data))
			encryptedSize = sizeof(packet->encrypted_data);

		if(!aesCipher.decrypt(packet->encrypted_data, encryptedSize, decryptedData)) {
			log(LL_Warning, "onPacketSelectServerResult: Could not decrypt TS_AC_SELECT_SERVER\n");
			closeSession();
			return;
		}

		if(decryptedData.size() < sizeof(uint64_t)) {
			log(LL_Warning,
			    "onPacketSelectServerResult: OTP size %d, expected <= %d\n",
			    (int) decryptedData.size(),
			    (int) sizeof(uint64_t));
			closeSession();
			return;
		}

		oneTimePassword = *reinterpret_cast<uint64_t*>(decryptedData.data());
		// pendingTimeBeforeServerMove = packetv2->pending_time;
	}

	const ServerConnectionInfo& selectedServerInfo = serverList.at(selectedServer);
	if(gameSession)
		gameSession->connect(selectedServerInfo.ip.c_str(), selectedServerInfo.port);
	normalDisconnect = true;
	closeSession();

	std::vector<ServerConnectionInfo> empty;
	serverList.swap(empty);
}

void ClientAuthSession::onPacketAccountName(const TS_AC_ACCOUNT_NAME* packet) {
	username = packet->account;
}

void ClientAuthSession::onPacketAuthResult(const TS_AC_RESULT* packet) {
	if(packet->request_msg_id == TS_CA_ACCOUNT::getId(packetVersion)) {
		onAuthResult((TS_ResultCode) packet->result, std::string());
	}
}

void ClientAuthSession::onPacketAuthStringResult(const TS_AC_RESULT_WITH_STRING* packet) {
	if(packet->request_msg_id == TS_CA_ACCOUNT::getId(packetVersion)) {
		onAuthResult((TS_ResultCode) packet->result, packet->string);
	}
}
