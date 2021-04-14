#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_AC_ACCOUNT_NAME_DEF(_) \
	_(def)(string)(account, 64) \
	_(impl)(string)(account, 64, version >= EPIC_9_6_6) \
	_(impl)(string)(account, 61, version < EPIC_9_6_6) \
	_(simple)(uint32_t, account_id) \
	_(endarray)(uint8_t, unknown1, version >= EPIC_9_6_3)

CREATE_PACKET(TS_AC_ACCOUNT_NAME, 10014, SessionType::AuthClient, SessionPacketOrigin::Server);

