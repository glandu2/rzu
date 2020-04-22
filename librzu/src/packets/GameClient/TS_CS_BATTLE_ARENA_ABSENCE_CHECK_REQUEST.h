#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST_DEF(_) \
	_(simple)(uint32_t, hCheckTarget)

// Since EPIC_8_1
CREATE_PACKET(TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST, 4717);
#undef TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST_DEF

