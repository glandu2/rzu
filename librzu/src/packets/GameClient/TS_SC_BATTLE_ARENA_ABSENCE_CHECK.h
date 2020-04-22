#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_BATTLE_ARENA_ABSENCE_CHECK_DEF(_) \
	_(simple)(ar_time_t, nLimitTime)

// Since EPIC_8_1
CREATE_PACKET(TS_SC_BATTLE_ARENA_ABSENCE_CHECK, 4718);
#undef TS_SC_BATTLE_ARENA_ABSENCE_CHECK_DEF

