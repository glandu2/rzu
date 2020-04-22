#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_RETRIEVE_CREATURE_DEF(_) \
	_(simple)(ar_handle_t, creature_card_handle)

// Since EPIC_7_3
CREATE_PACKET(TS_CS_RETRIEVE_CREATURE, 6004);
#undef TS_CS_RETRIEVE_CREATURE_DEF

