#pragma once

#include "Packet/PacketBaseMessage.h"

#include "Packet/PacketDeclaration.h"

#define TS_CA_IMBC_ACCOUNT_SIZE_PRE_EPIC_7_4 79

#define TS_IMBC_ACCOUNT_PASSWORD_PLAIN_DEF(_) \
	_(array)(uint8_t, password, 48)
CREATE_STRUCT(TS_IMBC_ACCOUNT_PASSWORD_PLAIN);

#define TS_IMBC_ACCOUNT_PASSWORD_AES_DEF(_) \
	_(simple)(uint32_t, password_size) \
	_(endarray)(uint8_t, password) /* At least 64 bytes, or more if needed */
CREATE_STRUCT(TS_IMBC_ACCOUNT_PASSWORD_AES);

#define TS_CA_IMBC_ACCOUNT_DEF(_) \
	_(def)(string)(account, 64) \
	_(impl)(string)(account, 64, version >= EPIC_9_6_6) \
	_(impl)(string)(account, 61, version >= EPIC_7_4 && version < EPIC_9_6_6) /* unchecked */ \
	_(impl)(string)(account, 24, version <  EPIC_7_4) \
	_(simple)(TS_IMBC_ACCOUNT_PASSWORD_PLAIN, passwordPlain, version < EPIC_8_1_1_RSA) \
	_(simple)(TS_IMBC_ACCOUNT_PASSWORD_AES, passwordAes, version >= EPIC_8_1_1_RSA)
CREATE_PACKET(TS_CA_IMBC_ACCOUNT, 10012, SessionType::AuthClient, SessionPacketOrigin::Client);

