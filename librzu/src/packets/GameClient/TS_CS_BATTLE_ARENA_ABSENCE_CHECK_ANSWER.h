#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER_DEF(_) \
	_(simple)(bool, bSuccess)

// Since EPIC_8_1
CREATE_PACKET(TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER, 4719);
#undef TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER_DEF

