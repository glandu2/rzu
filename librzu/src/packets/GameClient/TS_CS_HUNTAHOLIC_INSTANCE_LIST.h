#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_HUNTAHOLIC_INSTANCE_LIST_DEF(_) \
	_(simple)(int32_t, page)

CREATE_PACKET(TS_CS_HUNTAHOLIC_INSTANCE_LIST, 4000);
#undef TS_CS_HUNTAHOLIC_INSTANCE_LIST_DEF

