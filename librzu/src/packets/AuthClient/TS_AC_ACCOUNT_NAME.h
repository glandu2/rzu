#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_AC_ACCOUNT_NAME_DEF(_) \
	_(def)(string)(account, 64) \
	_(impl)(string)(account, 64, version >= EPIC_9_6_6) \
	_(impl)(string)(account, 61, version < EPIC_9_6_6) \
	_(simple)(uint32_t, user_no) \
	_(simple)(uint32_t, unknown) \
	_(endstring)(usertoken, true, version >= EPIC_9_6_3)

CREATE_PACKET(TS_AC_ACCOUNT_NAME, 10014, SessionType::AuthClient, SessionPacketOrigin::Server);

