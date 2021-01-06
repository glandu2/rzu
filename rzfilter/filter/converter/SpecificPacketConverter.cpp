#include "SpecificPacketConverter.h"
#include <functional>

#include "Cipher/DesPasswordCipher.h"
#include "Cipher/RzHashReversible256.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include <algorithm>

#include "AuthClient/TS_AC_ACCOUNT_NAME.h"
#include "AuthClient/TS_AC_AES_KEY_IV.h"
#include "AuthClient/TS_AC_RESULT.h"
#include "AuthClient/TS_AC_RESULT_WITH_STRING.h"
#include "AuthClient/TS_AC_SELECT_SERVER.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "AuthClient/TS_CA_DISTRIBUTION_INFO.h"
#include "AuthClient/TS_CA_IMBC_ACCOUNT.h"
#include "AuthClient/TS_CA_RSA_PUBLIC_KEY.h"
#include "AuthClient/TS_CA_VERSION.h"

#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_CS_CHECK_ILLEGAL_USER.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_CS_REGION_UPDATE.h"
#include "GameClient/TS_CS_REPORT.h"
#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_SC_CHANGE_LOCATION.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_MOVE.h"
#include "GameClient/TS_SC_MOVE_ACK.h"
#include "GameClient/TS_SC_NPC_TRADE_INFO.h"
#include "GameClient/TS_SC_STATE.h"
#include "GameClient/TS_SC_WEAR_INFO.h"

#include "GameClient/TS_SC_BONUS_EXP_JP.h"

bool SpecificPacketConverter::convertAuthPacketAndSend(IFilterEndpoint* client,
                                                       IFilterEndpoint* server,
                                                       const TS_MESSAGE* packet,
                                                       bool) {
	if(packet->id == TS_CA_VERSION::getId(client->getPacketVersion())) {
		TS_CA_VERSION pkt;
		if(packet->process(pkt, client->getPacketVersion())) {
			// Don't overwrite version if it is a request string and not a version
			if(isNormalVersion(pkt.szVersion)) {
				if(server->getPacketVersion() <= EPIC_3)
					pkt.szVersion = "200609280";
				else if(server->getPacketVersion() <= EPIC_9_1)
					pkt.szVersion = "200701120";
				else if(server->getPacketVersion() <= EPIC_9_6)
					pkt.szVersion = "201507080";
				else
					pkt.szVersion = GlobalCoreConfig::get()->client.authVersion;
			}

			server->sendPacket(pkt);
		}
	} else if(packet->id == TS_CA_RSA_PUBLIC_KEY::getId(client->getPacketVersion())) {
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
		} else {
			return true;
		}
	} else if(packet->id == TS_CA_ACCOUNT::getId(client->getPacketVersion())) {
		TS_CA_ACCOUNT pkt;
		std::vector<uint8_t> plainPassword;
		account.isBoraAccount = false;

		if(packet->process(pkt, client->getPacketVersion())) {
			if(client->getPacketVersion() >= EPIC_8_1_1_RSA) {
				if(pkt.account == "bora" && pkt.passwordAes.password_size == 0) {
					plainPassword =
					    Utils::convertToDataArray(pkt.passwordAes.password, sizeof(pkt.passwordAes.password));
					account.isBoraAccount = true;
				} else {
					if(!clientAesCipher.decrypt(
					       pkt.passwordAes.password,
					       Utils::clamp(
					           pkt.passwordAes.password_size, 0u, (unsigned int) sizeof(pkt.passwordAes.password)),
					       plainPassword)) {
						Object::logStatic(
						    Object::LL_Error, "rzfilter_version_converter", "Failed to decrypt AES password\n");
					}
				}
			} else {
				int size = (client->getPacketVersion() >= EPIC_5_1) ? 61 : 32;
				plainPassword.resize(size);
				DesPasswordCipher desCipher("MERONG");
				desCipher.decrypt(pkt.passwordDes.password, size);
				plainPassword.assign(pkt.passwordDes.password,
				                     std::find(pkt.passwordDes.password, pkt.passwordDes.password + size, '\0'));
			}

			account.useImbc = false;

			if(server->getPacketVersion() >= EPIC_8_1_1_RSA) {
				TS_CA_RSA_PUBLIC_KEY rsaKeyPkt;

				if(!serverRsaCipher.isInitialized())
					serverRsaCipher.generateKey();
				serverRsaCipher.getPemPublicKey(rsaKeyPkt.key);

				server->sendPacket(rsaKeyPkt);

				account.account = pkt.account;
				account.password = plainPassword;
				account.additionalInfos = pkt.additionalInfos;
			} else {
				int size = (server->getPacketVersion() >= EPIC_5_1) ? 61 : 32;
				DesPasswordCipher desCipher("MERONG");
				memset(pkt.passwordDes.password, 0, sizeof(pkt.passwordDes.password));
				memcpy(pkt.passwordDes.password,
				       plainPassword.data(),
				       std::min(sizeof(pkt.passwordDes.password), plainPassword.size()));
				desCipher.encrypt(pkt.passwordDes.password, size);

				server->sendPacket(pkt);
			}
		} else {
			return true;
		}
	} else if(packet->id == TS_CA_IMBC_ACCOUNT::getId(client->getPacketVersion())) {
		TS_CA_IMBC_ACCOUNT pkt;
		std::vector<uint8_t> plainPassword;

		if(packet->process(pkt, client->getPacketVersion())) {
			if(client->getPacketVersion() >= EPIC_8_1_1_RSA) {
				if(!clientAesCipher.decrypt(
				       pkt.passwordAes.password,
				       Utils::clamp(pkt.passwordAes.password_size, 0u, (unsigned int) sizeof(pkt.passwordAes.password)),
				       plainPassword)) {
					Object::logStatic(
					    Object::LL_Error, "rzfilter_version_converter", "Failed to decrypt AES password\n");
				}
			} else {
				plainPassword =
				    Utils::convertToDataArray(pkt.passwordPlain.password, sizeof(pkt.passwordPlain.password));
			}

			account.useImbc = true;

			if(server->getPacketVersion() >= EPIC_8_1_1_RSA) {
				TS_CA_RSA_PUBLIC_KEY rsaKeyPkt;

				if(!serverRsaCipher.isInitialized())
					serverRsaCipher.generateKey();
				serverRsaCipher.getPemPublicKey(rsaKeyPkt.key);

				server->sendPacket(rsaKeyPkt);

				account.account = pkt.account;
				account.password = plainPassword;
			} else {
				memcpy(pkt.passwordPlain.password,
				       plainPassword.data(),
				       std::min(sizeof(pkt.passwordPlain.password), plainPassword.size()));

				server->sendPacket(pkt);
			}
		} else {
			return true;
		}
	} else if(packet->id == TS_AC_AES_KEY_IV::getId(server->getPacketVersion())) {
		TS_AC_AES_KEY_IV pkt;
		if(packet->process(pkt, server->getPacketVersion())) {
			if(account.useImbc) {
				TS_CA_IMBC_ACCOUNT accountPkt;
				accountPkt.account = account.account;

				std::vector<uint8_t> aesKey;
				serverRsaCipher.privateDecrypt(pkt.data.data(), pkt.data.size(), aesKey);
				if(aesKey.size() == 32)
					serverAesCipher.init(aesKey.data());
				else
					Object::logStatic(
					    Object::LL_Error, "rzfilter_version_converter", "Bad AES key size: %d\n", (int) aesKey.size());

				std::vector<uint8_t> encryptedPassword;
				serverAesCipher.encrypt(account.password.data(), account.password.size(), encryptedPassword);
				memset(accountPkt.passwordAes.password, 0, sizeof(accountPkt.passwordAes.password));
				memcpy(accountPkt.passwordAes.password,
				       encryptedPassword.data(),
				       std::min(sizeof(accountPkt.passwordAes.password), encryptedPassword.size()));
				accountPkt.passwordAes.password_size = (uint32_t) encryptedPassword.size();

				server->sendPacket(accountPkt);

				memset(&account.password[0], 0, account.password.size());
			} else {
				TS_CA_ACCOUNT accountPkt;
				accountPkt.account = account.account;
				accountPkt.additionalInfos = account.additionalInfos;

				if(account.isBoraAccount) {
					accountPkt.passwordAes.password_size = 0;
					memset(accountPkt.passwordAes.password, 0, sizeof(accountPkt.passwordAes.password));
					memcpy(accountPkt.passwordAes.password,
					       account.password.data(),
					       std::min(sizeof(accountPkt.passwordAes.password), account.password.size()));
				} else {
					std::vector<uint8_t> aesKey;
					serverRsaCipher.privateDecrypt(pkt.data.data(), pkt.data.size(), aesKey);
					if(aesKey.size() == 32)
						serverAesCipher.init(aesKey.data());
					else
						Object::logStatic(Object::LL_Error,
						                  "rzfilter_version_converter",
						                  "Bad AES key size: %d\n",
						                  (int) aesKey.size());

					std::vector<uint8_t> encryptedPassword;
					serverAesCipher.encrypt(account.password.data(), account.password.size(), encryptedPassword);
					memset(accountPkt.passwordAes.password, 0, sizeof(accountPkt.passwordAes.password));
					memcpy(accountPkt.passwordAes.password,
					       encryptedPassword.data(),
					       std::min(sizeof(accountPkt.passwordAes.password), encryptedPassword.size()));
					accountPkt.passwordAes.password_size = (uint32_t) encryptedPassword.size();
				}

				server->sendPacket(accountPkt);

				memset(&account.password[0], 0, account.password.size());
			}
		} else {
			return true;
		}
	} else if(packet->id == TS_AC_ACCOUNT_NAME::getId(server->getPacketVersion())) {
		if(client->getPacketVersion() >= EPIC_9_4)
			return true;
	} else if(packet->id == TS_AC_SELECT_SERVER::getId(server->getPacketVersion())) {
		TS_AC_SELECT_SERVER pkt;
		uint64_t otp;

		if(packet->process(pkt, server->getPacketVersion())) {
			if(server->getPacketVersion() >= EPIC_8_1_1_RSA) {
				std::vector<uint8_t> plainOTP;
				if(!serverAesCipher.decrypt(pkt.encrypted_data,
				                            Utils::clamp(pkt.encrypted_data_size, 0, (int) sizeof(pkt.encrypted_data)),
				                            plainOTP)) {
					Object::logStatic(
					    Object::LL_Error, "rzfilter_version_converter", "Failed to decrypt AES OTP password\n");
				}
				if(plainOTP.size() >= sizeof(otp)) {
					otp = *reinterpret_cast<uint64_t*>(plainOTP.data());
				} else {
					Object::logStatic(Object::LL_Error,
					                  "rzfilter_version_converter",
					                  "Select server OTP wrong size: %d\n",
					                  (int) plainOTP.size());
					otp = 0;
				}
			} else {
				otp = pkt.one_time_key;
			}

			Object::logStatic(Object::LL_Debug, "rzfilter_version_converter", "OTP: 0x%08" PRIx64 "\n", otp);

			if(client->getPacketVersion() >= EPIC_8_1_1_RSA) {
				std::vector<uint8_t> encryptedOTP;
				std::vector<uint8_t> plainOTP;
				plainOTP.resize(sizeof(otp));
				memcpy(plainOTP.data(), &otp, sizeof(otp));

				clientAesCipher.encrypt(plainOTP.data(), plainOTP.size(), encryptedOTP);
				memcpy(
				    pkt.encrypted_data, encryptedOTP.data(), std::min(sizeof(pkt.encrypted_data), encryptedOTP.size()));
				pkt.encrypted_data_size = (uint32_t) encryptedOTP.size();

				client->sendPacket(pkt);
			} else {
				pkt.one_time_key = otp;
				client->sendPacket(pkt);
			}
		} else {
			return true;
		}
	} else if(packet->id == TS_AC_RESULT_WITH_STRING::getId(server->getPacketVersion())) {
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
		return true;
	}

	return false;
}

bool SpecificPacketConverter::convertGamePacketAndSend(IFilterEndpoint* target,
                                                       const TS_MESSAGE* packet,
                                                       int version,
                                                       bool) {
	if(packet->id == TS_CS_VERSION::getId(version)) {
		TS_CS_VERSION pkt;

		if(packet->process(pkt, version)) {
			// Don't overwrite version if it is a request string and not a version
			if(isNormalVersion(pkt.szVersion)) {
				if(target->getPacketVersion() <= EPIC_3)
					pkt.szVersion = "200609280";
				else if(target->getPacketVersion() <= EPIC_9_1)
					pkt.szVersion = "200701120";
				else if(target->getPacketVersion() <= EPIC_9_4)
					pkt.szVersion = "201507080";
				else if(target->getPacketVersion() <= EPIC_9_5_1)
					pkt.szVersion = "205001120";
				else if(target->getPacketVersion() <= EPIC_9_5_2)
					pkt.szVersion = "20180117";
				else if(target->getPacketVersion() <= EPIC_9_5_3)
					pkt.szVersion = "20181211";
				else if(target->getPacketVersion() <= EPIC_9_6_2)
					pkt.szVersion = "20190102";
				else
					pkt.szVersion = GlobalCoreConfig::get()->client.gameVersion;
			}

			if(target->getPacketVersion() >= EPIC_9_5_2) {
				RzHashReversible256::generatePayload(pkt);
			}

			target->sendPacket(pkt);

			// TS_CS_REPORT is required in 9.5.2+
			if(target->getPacketVersion() >= EPIC_9_5_2) {
				TS_CS_REPORT reportMsg;
				reportMsg.report =
				    "\x8D\x07\x72\xCA\x29\x47Windows (6.2.9200)|Intel(R) HD Graphics 4000Drv Version : 10.18.10.4425";
				target->sendPacket(reportMsg);
			}
		} else {
			return true;
		}
	} else if(packet->id == TS_CS_LOGIN::getId(version)) {
		TS_CS_LOGIN pkt;
		if(packet->process(pkt, version)) {
			if(target->getPacketVersion() >= EPIC_9_5_2 && version < EPIC_9_5_2)
				RzHashReversible256::generatePayload(pkt);

			target->sendPacket(pkt);
		} else {
			return true;
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
		} else {
			return true;
		}
	} else if(packet->id == TS_CS_CHECK_ILLEGAL_USER::getId(version)) {
		return false;
	} else if(packet->id == TS_SC_STATE::getId(version)) {
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
	} else if(packet->id == TS_SC_MOVE::getId(version)) {
		TS_SC_MOVE pkt;
		if(packet->process(pkt, version)) {
			startTime = Utils::getTimeInMsec();
		}
		return true;
	} else if(packet->id == TS_SC_MOVE_ACK::getId(version)) {
		TS_SC_MOVE_ACK pkt;
		if(packet->process(pkt, version)) {
			startTime = Utils::getTimeInMsec();
		}
		return true;
	} else if(packet->id == TS_CS_REGION_UPDATE::getId(version)) {
		TS_CS_REGION_UPDATE pkt;
		if(packet->process(pkt, version)) {
			if(version >= EPIC_9_3 && target->getPacketVersion() < EPIC_9_3) {
				pkt.update_time = static_cast<ar_time_t>(static_cast<uint32_t>(
				    (Utils::getTimeInMsec() - startTime) / 10 + (pkt.bIsStopMessage != 0 ? 10 : 0)));
			}
			target->sendPacket(pkt);
		} else {
			return true;
		}
	} else if(packet->id == TS_CS_REPORT::getId(version)) {
		if(target->getPacketVersion() >= EPIC_9_5_2) {
			// Already sent with version
			return false;
		} else {
			return true;
		}
	} else if(packet->id == TS_CS_ACCOUNT_WITH_AUTH::getId(version)) {
		TS_CS_ACCOUNT_WITH_AUTH pkt;
		if(packet->process(pkt, version)) {
			Object::logStatic(
			    Object::LL_Debug, "rzfilter_version_converter", "OTP: 0x%08" PRIx64 "\n", pkt.one_time_key);

			target->sendPacket(pkt);
		} else {
			return true;
		}
	} else {
		return true;
	}

	return false;
}

bool SpecificPacketConverter::isNormalVersion(const std::string& version) {
	for(char c : version) {
		if(!Utils::isdigit(c))
			return false;
	}

	return true;
}
