#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_DROP_QUEST_DEF(_) \
	_(simple)(int32_t, code)

CREATE_PACKET(TS_CS_DROP_QUEST, 603);
#undef TS_CS_DROP_QUEST_DEF

