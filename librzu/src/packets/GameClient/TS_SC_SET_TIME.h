#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_SET_TIME_DEF(_) \
	_(simple)(int32_t, gap)

CREATE_PACKET(TS_SC_SET_TIME, 10);
#undef TS_SC_SET_TIME_DEF

