#pragma once

#include "Packet/PacketDeclaration.h"
#include "TS_SC_BATTLE_ARENA_BATTLE_INFO.h"

#define TS_SC_BATTLE_ARENA_JOIN_QUEUE_DEF(_) \
	_(simple)(int32_t, nArenaID) \
	_(simple)(TS_BATTLE_GRADE, eGrade)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_BATTLE_ARENA_JOIN_QUEUE, 4702);
#undef TS_SC_BATTLE_ARENA_JOIN_QUEUE_DEF

