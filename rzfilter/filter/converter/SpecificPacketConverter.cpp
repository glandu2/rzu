#include "SpecificPacketConverter.h"
#include <functional>

#include "Cipher/DesPasswordCipher.h"
#include "Core/Utils.h"
#include <algorithm>

#include "AuthClient/TS_AC_ACCOUNT_NAME.h"
#include "AuthClient/TS_AC_AES_KEY_IV.h"
#include "AuthClient/TS_AC_RESULT.h"
#include "AuthClient/TS_AC_RESULT_WITH_STRING.h"
#include "AuthClient/TS_AC_SELECT_SERVER.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "AuthClient/TS_CA_DISTRIBUTION_INFO.h"
#include "AuthClient/TS_CA_RSA_PUBLIC_KEY.h"
#include "AuthClient/TS_CA_VERSION.h"

#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_CS_CHECK_ILLEGAL_USER.h"
#include "GameClient/TS_SC_STATE.h"
#include "GameClient/TS_SC_ENTER.h"

bool SpecificPacketConverter::convertAuthPacketAndSend(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, bool) {
	if(packet->id == TS_CA_VERSION::packetID) {
		TS_CA_VERSION pkt;
		if(packet->process(pkt, client->getPacketVersion())) {
			if(server->getPacketVersion() <= EPIC_3)
				pkt.szVersion = "200609280";
			else if(server->getPacketVersion() <= EPIC_9_1)
				pkt.szVersion = "200701120";
			else if(server->getPacketVersion() <= EPIC_9_4)
				pkt.szVersion = "201507080";
			else
				pkt.szVersion = "205001120";
			server->sendPacket(pkt);
		}
	} else if(packet->id == TS_CA_RSA_PUBLIC_KEY::packetID) {
		TS_CA_RSA_PUBLIC_KEY pkt;
		if(packet->process(pkt, client->getPacketVersion())) {
			RsaCipher clientRsaCipher;
			std::vector<uint8_t> aesKey;

			clientRsaCipher.loadKey(pkt.key);
			clientAesCipher.init();
			clientAesCipher.getKey(aesKey);

			TS_AC_AES_KEY_IV aesPkt;

			clientRsaCipher.publicEncrypt(aesKey.data(), aesKey.size(), aesPkt.data);
			memset(aesKey.data(), 0, aesKey.size());

			client->sendPacket(aesPkt);
		}
	} else if(packet->id == TS_CA_ACCOUNT::packetID) {
		TS_CA_ACCOUNT pkt;
		std::vector<uint8_t> plainPassword;

		if(packet->process(pkt, client->getPacketVersion())) {
			if(client->getPacketVersion() >= EPIC_8_1_1_RSA) {
				clientAesCipher.decrypt(pkt.passwordAes.password, pkt.passwordAes.password_size, plainPassword);
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
			serverRsaCipher.privateDecrypt(pkt.data.data(), pkt.data.size(), aesKey);
			serverAesCipher.init(aesKey.data());

			std::vector<uint8_t> encryptedPassword;
			serverAesCipher.encrypt(account.password.data(), account.password.size(), encryptedPassword);
			memset(accountPkt.passwordAes.password, 0, sizeof(accountPkt.passwordAes.password));
			memcpy(accountPkt.passwordAes.password, encryptedPassword.data(), encryptedPassword.size());
			accountPkt.passwordAes.password_size = (uint32_t)encryptedPassword.size();

			server->sendPacket(accountPkt);

			memset(&account.password[0], 0, account.password.size());
		}
	} else if(packet->id == TS_AC_ACCOUNT_NAME::packetID) {
		if(client->getPacketVersion() >= EPIC_9_4)
			return true;
	} else if(packet->id == TS_AC_SELECT_SERVER::packetID) {
		TS_AC_SELECT_SERVER pkt;
		uint64_t otp;

		if(packet->process(pkt, server->getPacketVersion())) {
			if(server->getPacketVersion() >= EPIC_8_1_1_RSA) {
				std::vector<uint8_t> plainOTP;
				serverAesCipher.decrypt(pkt.encrypted_data, pkt.encrypted_data_size, plainOTP);
				if(plainOTP.size() >= sizeof(otp)) {
					otp = *reinterpret_cast<uint64_t*>(plainOTP.data());
				} else {
					Object::logStatic(Object::LL_Error, "rzfilter_version_converter", "Select server OTP wrong size: %d\n",
					        (int)plainOTP.size());
					otp = 0;
				}
			} else {
				otp = pkt.one_time_key;
			}

			if(client->getPacketVersion() >= EPIC_8_1_1_RSA) {
				std::vector<uint8_t> encryptedOTP;
				std::vector<uint8_t> plainOTP;
				plainOTP.resize(sizeof(otp));
				memcpy(plainOTP.data(), &otp, sizeof(otp));

				clientAesCipher.encrypt(plainOTP.data(), plainOTP.size(), encryptedOTP);
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
		}
	} else {
		return true;
	}

	return false;
}

bool SpecificPacketConverter::convertGamePacketAndSend(IFilterEndpoint* target, const TS_MESSAGE* packet, int version, bool) {
	if(packet->id == TS_CS_VERSION::getId(version)) {
		TS_CS_VERSION pkt;
		if(packet->process(pkt, version)) {
			if(target->getPacketVersion() <= EPIC_3)
				pkt.szVersion = "200609280";
			else if(target->getPacketVersion() <= EPIC_9_1)
				pkt.szVersion = "200701120";
			else if(target->getPacketVersion() <= EPIC_9_4)
				pkt.szVersion = "201507080";
			else
				pkt.szVersion = "205001120";
			target->sendPacket(pkt);
		}
	} else if(packet->id == TS_SC_LOGIN_RESULT::getId(version)) {
		TS_SC_LOGIN_RESULT pkt;
		if(packet->process(pkt, version)) {
			uint16_t loginResult;
			if(version < EPIC_7_1) {
				loginResult = pkt.result ? TS_RESULT_SUCCESS : TS_RESULT_DB_ERROR;
			} else {
				loginResult = pkt.result;
			}

			if(target->getPacketVersion() < EPIC_7_1) {
				pkt.result = loginResult == TS_RESULT_SUCCESS;
			} else {
				pkt.result = loginResult;
			}

			target->sendPacket(pkt);
		}
	} else if(packet->id == TS_CS_CHECK_ILLEGAL_USER::packetID) {
		return false;
	} else if(packet->id == TS_SC_STATE::packetID) {
		TS_SC_STATE pkt;
		if(packet->process(pkt, version)) {
			// Epic 3 client crashes if the state code is not in db_tenacity(ascii).rdb
			if(target->getPacketVersion() <= EPIC_3)
				pkt.state_code = 1001;
			target->sendPacket(pkt);
			return false;
		} else {
			return true;
		}
	} else if(packet->id == TS_SC_ENTER::getId(version)) {
		TS_SC_ENTER pkt;
		if(packet->process(pkt, version)) {
			if(pkt.objType == EOT_Player) {
				if(version >= EPIC_4_1) {
					pkt.playerInfo.energy = pkt.playerInfo.creatureInfo.energy;
				} else {
					pkt.playerInfo.creatureInfo.energy = pkt.playerInfo.energy;
				}
			}
			target->sendPacket(pkt);
			return false;
		} else {
			return true;
		}
	} else {
		return true;
	}

	return false;
}
