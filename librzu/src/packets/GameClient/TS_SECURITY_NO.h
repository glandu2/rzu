#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SECURITY_NO_DEF(_) \
	_(string)(account, 64, version >= EPIC_9_6_7) \
	_(simple)(int32_t, mode) \
	_(string)(security_no, 19, version < EPIC_9_6_7) \
	_(simple)(int32_t, result, version >= EPIC_9_6_7) \
	_(simple)(int32_t, security_no_1, version >= EPIC_9_6_7) \
	_(simple)(int32_t, security_no_2, version >= EPIC_9_6_7)

#define TS_SECURITY_NO_ID(X) \
	X(9005, version < EPIC_9_6_3) \
	X(8105, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SECURITY_NO, SessionType::GameClient, SessionPacketOrigin::Any);
#undef TS_SECURITY_NO_DEF

