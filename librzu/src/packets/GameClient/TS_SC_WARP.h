#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_SC_WARP_DEF(_) \
	_(simple)(float, x) \
	_(simple)(float, y) \
	_(simple)(float, z) \
	_(simple)(int8_t, layer)

#define TS_SC_WARP_ID(X) \
	X(12, version < EPIC_9_6_3) \
	X(1012, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_WARP, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_WARP_DEF

