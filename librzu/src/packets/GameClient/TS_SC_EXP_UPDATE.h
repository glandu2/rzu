#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_EXP_UPDATE_DEF(_) \
	_(simple) (ar_handle_t, handle) \
	_(simple) (uint64_t, exp) \
	_(def)(simple) (uint64_t, jp) \
	_(impl)(simple)(uint64_t, jp, version >= EPIC_7_3) \
	_(impl)(simple)(uint32_t, jp, version < EPIC_7_3)

CREATE_PACKET(TS_SC_EXP_UPDATE, 1003);
#undef TS_SC_EXP_UPDATE_DEF

