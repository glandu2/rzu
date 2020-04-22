#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_COMPETE_END_DEF(_) \
	_(simple)(int8_t, compete_type) \
	_(simple)(int8_t, end_type) \
	_(string)(winner, 31) \
	_(string)(loser, 31)

CREATE_PACKET(TS_SC_COMPETE_END, 4506);
#undef TS_SC_COMPETE_END_DEF

