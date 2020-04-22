#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_ITEM_WEAR_INFO_DEF(_) \
	_(simple)(ar_handle_t, item_handle) \
	_(simple)(int16_t, wear_position) \
	_(simple)(ar_handle_t, target_handle) \
	_(simple)(int32_t, enhance) \
	_(simple)(int8_t, elemental_effect_type, version >= EPIC_6_1) \
	_(simple)(int32_t, appearance_code, version >= EPIC_7_4)

CREATE_PACKET(TS_SC_ITEM_WEAR_INFO, 287);
#undef TS_SC_ITEM_WEAR_INFO_DEF

