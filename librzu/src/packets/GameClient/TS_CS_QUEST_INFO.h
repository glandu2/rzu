#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_QUEST_INFO_DEF(_) \
	_(simple)(int32_t, code)

// Since EPIC_6_3
CREATE_PACKET(TS_CS_QUEST_INFO, 604);
#undef TS_CS_QUEST_INFO_DEF

