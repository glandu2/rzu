#include "AuthClientSession.h"
#include "Cipher/RsaCipher.h"
#include "Core/Utils.h"
#include "GlobalConfig.h"
#include <algorithm>
#include <vector>

#include "AuthClient/TS_AC_AES_KEY_IV.h"
#include "AuthClient/TS_AC_RESULT.h"
#include "AuthClient/TS_AC_SELECT_SERVER.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "AuthClient/TS_CA_RSA_PUBLIC_KEY.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "AuthClient/TS_CA_SERVER_LIST.h"
#include "AuthClient/TS_CA_VERSION.h"

AuthClientSession::AuthClientSession()
    : EncryptedSession<PacketSession>(
          SessionType::AuthClient, SessionPacketOrigin::Server, CONFIG_GET()->generalConfig.epic.get()) {}

AuthClientSession::~AuthClientSession() {}

EventChain<PacketSession> AuthClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	packet_type_id_t packetType = PacketMetadata::convertPacketIdToTypeId(
	    packet->id, SessionType::AuthClient, SessionPacketOrigin::Client, packetVersion);
	switch(packetType) {
		case TS_CA_VERSION::packetID:
			packet->process(this, &AuthClientSession::onVersion, packetVersion);
			break;

		case TS_CA_RSA_PUBLIC_KEY::packetID:
			packet->process(this, &AuthClientSession::onRsaKey, packetVersion);
			break;

		case TS_CA_ACCOUNT::packetID:
			packet->process(this, &AuthClientSession::onAccount, packetVersion);
			break;

		case TS_CA_SERVER_LIST::packetID:
			packet->process(this, &AuthClientSession::onServerList, packetVersion);
			break;

		case TS_CA_SELECT_SERVER::packetID:
			packet->process(this, &AuthClientSession::onSelectServer, packetVersion);
			break;

		case 9999:
			break;

		default:
			log(LL_Debug, "Unknown packet ID: %d, size: %d\n", packet->id, packet->size);
			break;
	}

	return PacketSession::onPacketReceived(packet);
}

void AuthClientSession::onVersion(const TS_CA_VERSION* packet) {
	log(LL_Info, "Received login from client\n");
}

void AuthClientSession::onRsaKey(const TS_CA_RSA_PUBLIC_KEY* packet) {
	RsaCipher clientRsaCipher;
	std::vector<uint8_t> aesKey;

	log(LL_Info, "Received RSA key from client\n");

	clientRsaCipher.loadKey(packet->key);
	clientAesCipher.init();
	clientAesCipher.getKey(aesKey);

	TS_AC_AES_KEY_IV aesPkt;

	clientRsaCipher.publicEncrypt(aesKey.data(), aesKey.size(), aesPkt.data);
	memset(aesKey.data(), 0, aesKey.size());

	sendPacket(aesPkt);
}

void AuthClientSession::onAccount(const TS_CA_ACCOUNT* packet) {
	TS_AC_RESULT result;

	log(LL_Info, "Received login for account %s (accepting anything)\n", packet->account.c_str());

	account = packet->account;

	result.request_msg_id = packet->getReceivedId();
	result.result = 0;
	result.login_flag = LSF_EULA_ACCEPTED;
	sendPacket(result);
}

void AuthClientSession::onServerList(const TS_CA_SERVER_LIST* packet) {
	TS_AC_SERVER_LIST serverListPacket;

	log(LL_Info, "Received server list query\n");

	serverListPacket.servers.emplace_back();

	TS_SERVER_INFO& serverData = serverListPacket.servers.back();

	std::string listenIp = CONFIG_GET()->client.listener.listenIp.get();

	serverData.server_idx = 1;
	serverData.server_port = CONFIG_GET()->client.gamePort.get();
	serverData.is_adult_server = false;
	serverData.server_ip = CONFIG_GET()->client.gameExternalIp.get();
	serverData.server_name = "rzclientreconnect";
	serverData.server_screenshot_url = "about:blank";
	serverData.user_ratio = 0;

	serverListPacket.last_login_server_idx = serverData.server_idx;

	sendPacket(serverListPacket);
}

void AuthClientSession::onSelectServer(const TS_CA_SELECT_SERVER* packet) {
	TS_AC_SELECT_SERVER selectServerPacket;

	log(LL_Info, "Received server selection, client will move to game client session\n");

	selectServerPacket.result = 0;
	selectServerPacket.one_time_key = 0;
	selectServerPacket.pending_time = 0;

	std::vector<uint8_t> encryptedOneTimeKey;

	clientAesCipher.encrypt(reinterpret_cast<const uint8_t*>(&selectServerPacket.one_time_key),
	                        sizeof(selectServerPacket.one_time_key),
	                        encryptedOneTimeKey);
	memcpy(selectServerPacket.encrypted_data,
	       encryptedOneTimeKey.data(),
	       std::min(sizeof(selectServerPacket.encrypted_data), encryptedOneTimeKey.size()));
	selectServerPacket.encrypted_data_size = static_cast<int32_t>(encryptedOneTimeKey.size());

	sendPacket(selectServerPacket);
}
