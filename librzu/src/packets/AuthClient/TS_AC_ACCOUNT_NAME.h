#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_AC_ACCOUNT_NAME_DEF(_) \
	_(string)(account, 61) \
	_(simple)(uint32_t, account_id) \
	_(simple)(uint16_t, unknown1, version >= EPIC_9_6_3) \
	_(simple)(uint16_t, unknown2, version >= EPIC_9_6_3) \
	_(string)(unknown2_hex, 96, version >= EPIC_9_6_3)

CREATE_PACKET(TS_AC_ACCOUNT_NAME, 10014, SessionType::AuthClient, SessionPacketOrigin::Server);

