#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS_DEF(_) \
	_(simple)(int32_t, nReadyState)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS, 4709);
#undef TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS_DEF

