#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_CHANGE_ALIAS_DEF(_) \
	_(string)(alias, 19)

// Since EPIC_8_1
CREATE_PACKET(TS_CS_CHANGE_ALIAS, 31);
#undef TS_CS_CHANGE_ALIAS_DEF

