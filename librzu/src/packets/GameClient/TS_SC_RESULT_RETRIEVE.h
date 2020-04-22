#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_RESULT_RETRIEVE_DEF(_) \
	_(simple)(int8_t, result)

// Since EPIC_7_3
CREATE_PACKET(TS_SC_RESULT_RETRIEVE, 6005);
#undef TS_SC_RESULT_RETRIEVE_DEF

