#pragma once

#include "Packet/PacketDeclaration.h"

// Used for epic autodetection
#define TS_CA_ACCOUNT_SIZE_PRE_EPIC_5_2 58

enum TS_ADDITIONAL_INFO_TYPE :  int8_t
{
	TAIT_BLACKBOX = 0,
	TAIT_MACADDRESS = 1
};

#define TS_ACCOUNT_ADDITIONAL_INFO_DEF(_) \
	_(simple)(TS_ADDITIONAL_INFO_TYPE, type) \
	_(count)(uint16_t, data) \
	_(dynarray)(uint8_t, data)
CREATE_STRUCT(TS_ACCOUNT_ADDITIONAL_INFO);

#define TS_ACCOUNT_PASSWORD_DES_DEF(_) \
	_(def)(array)(uint8_t, password, 61) \
	_(impl)(array)(uint8_t, password, 61, version >= EPIC_5_2) \
	_(impl)(array)(uint8_t, password, 32, version <  EPIC_5_2)
CREATE_STRUCT(TS_ACCOUNT_PASSWORD_DES);

#define TS_ACCOUNT_PASSWORD_AES_DEF(_) \
	_(simple)(uint32_t, password_size) \
	_(def)(array)(uint8_t, password, 516) \
	_(impl)(array)(uint8_t, password, 516, version >= EPIC_9_6_6 && version < EPIC_9_6_7) \
	_(impl)(array)(uint8_t, password, 77, version < EPIC_9_6_6) \
	_(endarray)(uint8_t, password_dyn, version >= EPIC_9_6_7)
CREATE_STRUCT(TS_ACCOUNT_PASSWORD_AES);

#define TS_CA_ACCOUNT_DEF(_) \
	_(def)(string)(account, 64) \
	_(impl)(string)(account, 56, version >= EPIC_9_6_6) \
	_(impl)(string)(account, 61, version >= EPIC_5_2 && version < EPIC_9_6_6) \
	_(impl)(string)(account, 19, version <  EPIC_5_2) \
	_(array)(uint8_t, mac_stamp, 8, version >= EPIC_9_6_6) \
	_(simple)(TS_ACCOUNT_PASSWORD_DES, passwordDes, version < EPIC_8_1_1_RSA) \
	_(simple)(TS_ACCOUNT_PASSWORD_AES, passwordAes, version >= EPIC_8_1_1_RSA) \
	_(endarray)(TS_ACCOUNT_ADDITIONAL_INFO, additionalInfos, version < EPIC_9_6_6)
CREATE_PACKET(TS_CA_ACCOUNT, 10010, SessionType::AuthClient, SessionPacketOrigin::Client);

