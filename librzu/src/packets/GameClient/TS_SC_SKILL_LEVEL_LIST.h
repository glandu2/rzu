#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SKILL_LEVEL_INFO_DEF(_) \
	_(simple)(int32_t, skill_id) \
	_(simple)(int8_t, skill_level)

CREATE_STRUCT(TS_SKILL_LEVEL_INFO);
#undef TS_SKILL_LEVEL_INFO_DEF

#define TS_SC_SKILL_LEVEL_LIST_DEF(_) \
	_(count)(uint16_t, skill_levels) \
	_(dynarray)(TS_SKILL_LEVEL_INFO, skill_levels)

// Since EPIC_7_3
CREATE_PACKET(TS_SC_SKILL_LEVEL_LIST, 451);
#undef TS_SC_SKILL_LEVEL_LIST_DEF

