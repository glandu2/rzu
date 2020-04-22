#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_ITEM_COOL_TIME_DEF(_) \
	_(def)(array) (ar_time_t, cool_time, 40) \
	_(impl)(array)(ar_time_t, cool_time, 40, version >= EPIC_6_2) \
	_(impl)(array)(ar_time_t, cool_time, 20, version < EPIC_6_2)

CREATE_PACKET(TS_SC_ITEM_COOL_TIME, 217);
#undef TS_SC_ITEM_COOL_TIME_DEF

