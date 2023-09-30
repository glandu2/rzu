#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_GAME_TIME_DEF(_)

#define TS_CS_GAME_TIME_ID(X) \
	X(1100, version < EPIC_9_6_3) \
	X(2100, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_GAME_TIME, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_GAME_TIME_DEF

