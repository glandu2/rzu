#ifndef PACKETS_TS_SC_WEAR_INFO_H
#define PACKETS_TS_SC_WEAR_INFO_H

#include "Packet/PacketDeclaration.h"

#define TS_SC_WEAR_INFO_DEF(_) \
	_(simple) (uint32_t, handle) \
	_(def)(array)(uint32_t, item_code, 32) \
	  _(impl)(array)(uint32_t, item_code, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, item_code, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, item_code, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, item_code, 32, version >= EPIC_9_6) \
	_(def)(array)(uint32_t, item_enhance, 32) \
	  _(impl)(array)(uint32_t, item_enhance, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, item_enhance, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, item_enhance, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, item_enhance, 32, version >= EPIC_9_6) \
	_(def)(array)(uint32_t, item_level, 32) \
	  _(impl)(array)(uint32_t, item_level, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, item_level, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, item_level, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, item_level, 32, version >= EPIC_9_6) \
	_(def)(array)(uint8_t, elemental_effect_type, 32) \
	  _(impl)(array)(uint8_t, elemental_effect_type, 24, version >= EPIC_6_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint8_t, elemental_effect_type, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint8_t, elemental_effect_type, 32, version >= EPIC_9_6) \
	_(def)(array)(uint32_t, appearance_code, 32) \
	  _(impl)(array)(uint32_t, appearance_code, 24, version >= EPIC_7_4 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, appearance_code, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, appearance_code, 32, version >= EPIC_9_6) \
	_(padmarker)(padding) \
	_(simple)(uint8_t, booster_code, version >= EPIC_9_3 && version < EPIC_9_5, 0) \
	_(simple)(uint8_t, emblem_code, version >= EPIC_9_3 && version < EPIC_9_5, 0) \
	_(pad)(8, padding, version >= EPIC_9_3 && version < EPIC_9_5)

CREATE_PACKET(TS_SC_WEAR_INFO, 202);

#endif // PACKETS_TS_SC_WEAR_INFO_H
