#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_ACCOUNT_WITH_AUTH_DEF(_) \
	_(def)(string)(account, 64) \
	_(impl)(string)(account, 64, version >= EPIC_9_6_6) \
	_(impl)(string)(account, 61, version >= EPIC_5_2 && version < EPIC_9_6_6) \
	_(impl)(string)(account, 19, version <  EPIC_5_2) \
	_(simple)(uint64_t, one_time_key)

#define TS_CS_ACCOUNT_WITH_AUTH_ID(X) \
	X(2005, version < EPIC_9_6_3) \
	X(2405, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_ACCOUNT_WITH_AUTH, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_ACCOUNT_WITH_AUTH_DEF

