#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1


#define TS_SC_WEAR_INFO__ITEM_WEAR_INFO_DEF(_) \
    _(simple) (uint8_t, item_type) \
    _(simple) (uint8_t, item_unknown1) \
    _(simple) (uint8_t, item_enhance) \
    _(simple) (uint8_t, item_unknown2) \

CREATE_STRUCT(TS_SC_WEAR_INFO__ITEM_WEAR_INFO);
#undef TS_SC_WEAR_INFO__ITEM_WEAR_INFO_DEF // struct is 304 bytes long in epic2


#define TS_SC_WEAR_INFO_DEF(_) \
	_(simple) (ar_handle_t, handle) \
	_(def)(array)(uint32_t, item_code, 32) \
	  _(impl)(array)(uint32_t, item_code, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, item_code, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, item_code, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, item_code, 32, version >= EPIC_9_6 && version < EPIC_9_7_2) \
	  _(impl)(array)(uint32_t, item_code, 10, version >= EPIC_9_7_2) \
	_(def)(array)(uint32_t, item_enhance, 32) \
	  _(impl)(array)(uint32_t, item_enhance, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, item_enhance, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, item_enhance, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, item_enhance, 32, version >= EPIC_9_6 && version < EPIC_9_7_2) \
	_(def)(array)(uint32_t, item_level, 32) \
	  _(impl)(array)(uint32_t, item_level, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, item_level, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, item_level, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, item_level, 32, version >= EPIC_9_6 && version < EPIC_9_7_2) \
	_(def)(array)(uint8_t, elemental_effect_type, 32) \
	  _(impl)(array)(uint8_t, elemental_effect_type, 24, version >= EPIC_6_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint8_t, elemental_effect_type, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint8_t, elemental_effect_type, 32, version >= EPIC_9_6 && version < EPIC_9_7_2) \
	_(def)(array)(uint32_t, appearance_code, 32) \
	  _(impl)(array)(uint32_t, appearance_code, 24, version >= EPIC_7_4 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, appearance_code, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, appearance_code, 32, version >= EPIC_9_6 && version < EPIC_9_7_2) \
	_(array)(TS_SC_WEAR_INFO__ITEM_WEAR_INFO, item_details, 10, version >= EPIC_9_7_2) \
	_(padmarker)(padding) \
	_(simple)(uint8_t, booster_code, version >= EPIC_9_3 && version < EPIC_9_5, 0) \
	_(simple)(uint8_t, emblem_code, version >= EPIC_9_3 && version < EPIC_9_5, 0) \
	_(pad)(8, padding, version >= EPIC_9_3 && version < EPIC_9_5)

#define TS_SC_WEAR_INFO_ID(X) \
	X(202, version < EPIC_9_6_3) \
	X(1202, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_WEAR_INFO, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_WEAR_INFO_DEF

