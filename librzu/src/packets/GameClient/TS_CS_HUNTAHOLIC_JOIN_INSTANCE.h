#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_HUNTAHOLIC_JOIN_INSTANCE_DEF(_) \
	_(simple)(int32_t, instance_no) \
	_(string)(password, 17)

CREATE_PACKET(TS_CS_HUNTAHOLIC_JOIN_INSTANCE, 4004);
#undef TS_CS_HUNTAHOLIC_JOIN_INSTANCE_DEF

