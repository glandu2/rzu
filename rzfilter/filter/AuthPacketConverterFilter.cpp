#include "AuthPacketConverterFilter.h"
#include <functional>

#include "Cipher/DesPasswordCipher.h"
#include "Core/Utils.h"
#include <algorithm>

#include "AuthClient/TS_AC_ACCOUNT_NAME.h"
#include "AuthClient/TS_AC_AES_KEY_IV.h"
#include "AuthClient/TS_AC_RESULT.h"
#include "AuthClient/TS_AC_RESULT_WITH_STRING.h"
#include "AuthClient/TS_AC_SELECT_SERVER.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_AC_UPDATE_PENDING_TIME.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "AuthClient/TS_CA_DISTRIBUTION_INFO.h"
#include "AuthClient/TS_CA_IMBC_ACCOUNT.h"
#include "AuthClient/TS_CA_OTP_ACCOUNT.h"
#include "AuthClient/TS_CA_RSA_PUBLIC_KEY.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "AuthClient/TS_CA_SERVER_LIST.h"
#include "AuthClient/TS_CA_VERSION.h"

#define PACKET_TO_JSON(type_) \
	case type_::packetID: \
	(void)(sizeof(&type_::getSize)); \
	return sendPacket<type_>(target, packet, version);

AuthPacketConverterFilter::AuthPacketConverterFilter(AuthPacketConverterFilter *)
{
}

AuthPacketConverterFilter::~AuthPacketConverterFilter()
{
}

bool AuthPacketConverterFilter::onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType) {
	if(serverType == ST_Auth)
		return handleAuthPacket(client, server, packet, true);
	else
		return true;
}

bool AuthPacketConverterFilter::onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType) {
	if(serverType == ST_Auth)
		return handleAuthPacket(client, server, packet, false);
	else
		return true;
}

template<typename Packet>
bool sendPacket(IFilterEndpoint* target, const TS_MESSAGE* packet, int version, std::function<void(Packet*,int)> modifier = std::function<void(Packet*,int)>()) {
	Packet pkt = {0};
	if(packet->process(pkt, version)) {
		if(packet->id != Packet::getId(version))
			Object::logStatic(Object::LL_Warning, "rzfilter_version_converter", "Packet %s id mismatch, got %d, expected %d for version 0x%06x\n",
			        Packet::getName(),
			        packet->id,
			        Packet::getId(version),
			        version);
		if(modifier)
			modifier(&pkt, target->getPacketVersion());
		target->sendPacket(pkt);
		return false; //packet sent, no need to forward the original
	} else {
		Object::logStatic(Object::LL_Warning, "rzfilter_version_converter", "Can't parse packet id %d with version 0x%X\n", packet->id, version);
		return true; //packet not sent, need to forward the original
	}
}

bool AuthPacketConverterFilter::handleAuthPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, bool isServerMsg) {
	if(packet->id == TS_CA_VERSION::packetID) {
		sendPacket<TS_CA_VERSION>(server, packet, client->getPacketVersion(), [](TS_CA_VERSION* packet, int version) {
			if(version <= EPIC_3)
				packet->szVersion = "200609280";
			else if(version <= EPIC_9_1)
				packet->szVersion = "200701120";
			else if(version <= EPIC_9_4)
				packet->szVersion = "201507080";
			else
				packet->szVersion = "205001120";
		});
	} else if(packet->id == TS_CA_RSA_PUBLIC_KEY::packetID) {
		TS_CA_RSA_PUBLIC_KEY pkt;
		if(packet->process(pkt, client->getPacketVersion())) {
			RsaCipher clientRsaCipher;
			std::vector<uint8_t> aesKey;

			clientRsaCipher.loadKey(pkt.key);
			clientAesCipher.init();
			clientAesCipher.getKey(aesKey);

			TS_AC_AES_KEY_IV aesPkt;

			clientRsaCipher.publicEncrypt(aesKey, aesPkt.data);
			memset(aesKey.data(), 0, aesKey.size());

			client->sendPacket(aesPkt);
		}
	} else if(packet->id == TS_CA_ACCOUNT::packetID) {
		TS_CA_ACCOUNT pkt;
		std::vector<uint8_t> plainPassword;

		if(packet->process(pkt, client->getPacketVersion())) {
			if(client->getPacketVersion() >= EPIC_8_1_1_RSA) {
				clientAesCipher.decrypt(Utils::convertToDataArray(pkt.passwordAes.password, pkt.passwordAes.password_size), plainPassword);
			} else {
				int size = (client->getPacketVersion() >= EPIC_5_1)? 61 : 32;
				plainPassword.resize(size);
				DesPasswordCipher desCipher("MERONG");
				desCipher.decrypt(pkt.passwordDes.password, size);
				plainPassword.assign(pkt.passwordDes.password, std::find(pkt.passwordDes.password, pkt.passwordDes.password + size, '\0'));
			}

			if(server->getPacketVersion() >= EPIC_8_1_1_RSA) {
				TS_CA_RSA_PUBLIC_KEY rsaKeyPkt;

				serverRsaCipher.generateKey();
				serverRsaCipher.getPemPublicKey(rsaKeyPkt.key);

				server->sendPacket(rsaKeyPkt);

				account.account = pkt.account;
				account.password = plainPassword;
				account.additionalInfos = pkt.additionalInfos;
			} else {
				int size = (server->getPacketVersion() >= EPIC_5_1)? 61 : 32;
				DesPasswordCipher desCipher("MERONG");
				memset(pkt.passwordDes.password, 0, sizeof(pkt.passwordDes.password));
				memcpy(pkt.passwordDes.password, plainPassword.data(), plainPassword.size());
				desCipher.encrypt(pkt.passwordDes.password, size);

				server->sendPacket(pkt);
			}
		}
	} else if(packet->id == TS_AC_AES_KEY_IV::packetID) {
		TS_AC_AES_KEY_IV pkt;
		if(packet->process(pkt, server->getPacketVersion())) {
			TS_CA_ACCOUNT accountPkt;
			accountPkt.account = account.account;
			accountPkt.additionalInfos = account.additionalInfos;

			std::vector<uint8_t> aesKey;
			serverRsaCipher.privateDecrypt(pkt.data, aesKey);
			serverAesCipher.init(aesKey.data());

			std::vector<uint8_t> encryptedPassword;
			serverAesCipher.encrypt(account.password, encryptedPassword);
			memset(accountPkt.passwordAes.password, 0, sizeof(accountPkt.passwordAes.password));
			memcpy(accountPkt.passwordAes.password, encryptedPassword.data(), encryptedPassword.size());
			accountPkt.passwordAes.password_size = (uint32_t)encryptedPassword.size();

			server->sendPacket(accountPkt);

			memset(&account.password[0], 0, account.password.size());
		}
	} else if(packet->id == TS_AC_ACCOUNT_NAME::packetID) {
		if(client->getPacketVersion() >= EPIC_9_4)
			sendPacket<TS_AC_ACCOUNT_NAME>(client, packet, server->getPacketVersion());
	} else if(packet->id == TS_AC_SELECT_SERVER::packetID) {
		TS_AC_SELECT_SERVER pkt;
		uint64_t otp;

		if(packet->process(pkt, server->getPacketVersion())) {
			if(server->getPacketVersion() >= EPIC_8_1_1_RSA) {
				std::vector<uint8_t> plainOTP;
				serverAesCipher.decrypt(Utils::convertToDataArray(pkt.encrypted_data, pkt.encrypted_data_size), plainOTP);
				if(plainOTP.size() >= sizeof(otp)) {
					otp = *reinterpret_cast<uint64_t*>(plainOTP.data());
				} else {
					Object::logStatic(Object::LL_Error, "rzfilter_version_converter", "Select server OTP wrong size: %d\n",
					        (int)plainOTP.size());
				}
			} else {
				otp = pkt.one_time_key;
			}

			if(client->getPacketVersion() >= EPIC_8_1_1_RSA) {
				std::vector<uint8_t> encryptedOTP;
				std::vector<uint8_t> plainOTP;
				plainOTP.resize(sizeof(otp));
				memcpy(plainOTP.data(), &otp, sizeof(otp));

				clientAesCipher.encrypt(plainOTP, encryptedOTP);
				memcpy(pkt.encrypted_data, encryptedOTP.data(), encryptedOTP.size());
				pkt.encrypted_data_size = (uint32_t)encryptedOTP.size();

				client->sendPacket(pkt);
			} else {
				pkt.one_time_key = otp;
				client->sendPacket(pkt);
			}
		}
	} else if(packet->id == TS_AC_RESULT_WITH_STRING::packetID) {
		TS_AC_RESULT_WITH_STRING pkt;
		if(packet->process(pkt, server->getPacketVersion())) {
			if(client->getPacketVersion() >= EPIC_8_1) {
				client->sendPacket(pkt);
			} else {
				TS_AC_RESULT resultPkt;
				resultPkt.request_msg_id = pkt.request_msg_id;
				resultPkt.result = pkt.result;
				resultPkt.login_flag = pkt.login_flag;
				client->sendPacket(resultPkt);
			}
		} else {
			return true;
		}
	} else {
		int version = isServerMsg ? server->getPacketVersion() : client->getPacketVersion();
		IFilterEndpoint* target = isServerMsg ? client : server;
		switch(packet->id) {
			PACKET_TO_JSON(TS_AC_RESULT);
			PACKET_TO_JSON(TS_CA_SERVER_LIST);
			PACKET_TO_JSON(TS_AC_SERVER_LIST);
			PACKET_TO_JSON(TS_CA_SELECT_SERVER);
			PACKET_TO_JSON(TS_AC_UPDATE_PENDING_TIME);
			PACKET_TO_JSON(TS_CA_DISTRIBUTION_INFO);
			PACKET_TO_JSON(TS_CA_IMBC_ACCOUNT);
			PACKET_TO_JSON(TS_CA_OTP_ACCOUNT);
			case 9999: break;
			default:
				Object::logStatic(Object::LL_Warning, "rzfilter_version_converter", "auth packet id %d unknown\n", packet->id);
				break;
		}

		return true;
	}

	return false;
}

IFilter *createFilter(IFilter *oldFilter)
{
	Object::logStatic(Object::LL_Info, "rzfilter_version_converter", "Loaded filter from data: %p\n", oldFilter);
	return new AuthPacketConverterFilter((AuthPacketConverterFilter*)oldFilter);
}

void destroyFilter(IFilter *filter)
{
	delete filter;
}
