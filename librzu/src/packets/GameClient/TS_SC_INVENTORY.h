#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_ITEM_SOCKETS_DEF(_) \
	_(array)  (int32_t, socket, 4) \
	_(array)  (int32_t, dummy_socket, 2, version < EPIC_6_1, {0})
CREATE_STRUCT(TS_ITEM_SOCKETS);
#undef TS_ITEM_SOCKETS_DEF

#define TS_ITEM_AWAKEN_OPTION_DEF(_) \
	_(def)(array) (uint32_t, value, 7) \
	  _(impl)(array) (uint32_t, value, 5, version < EPIC_9_8_0) \
	  _(impl)(array) (uint32_t, value, 7, version >= EPIC_9_8_0) \
	_(def)(array) (int32_t, data, 7) \
	  _(impl)(array) (int32_t, data, 5, version < EPIC_9_8_0) \
	  _(impl)(array) (float, data, 7, version >= EPIC_9_8_0)
CREATE_STRUCT(TS_ITEM_AWAKEN_OPTION);
#undef TS_ITEM_AWAKEN_OPTION_DEF

#define TS_ITEM_RANDOM_STATS_DEF(_) \
	_(def)(array) (uint32_t, type, 10) \
	  _(impl)(array)(uint32_t, type, 10, version < EPIC_9_6_1) \
	  _(impl)(array)(uint32_t, type, 7, version >= EPIC_9_6_1 && version < EPIC_9_8_1) \
	  _(impl)(array)(uint16_t, type, 7, version >= EPIC_9_8_1) \
	_(def)(array) (float, value_1, 10) \
	  _(impl)(array)(int64_t, value_1, 10, version < EPIC_9_6_1) \
	  _(impl)(array)(int64_t, value_1, 7, version >= EPIC_9_6_1 && version < EPIC_9_6_7) \
	  _(impl)(array)(float, value_1, 7, version >= EPIC_9_6_7 && version < EPIC_9_8_1) \
	  _(impl)(array)(uint32_t, value_1, 7, version >= EPIC_9_8_1) \
	_(def)(array) (float, value_2, 10) \
	  _(impl)(array)(int64_t, value_2, 10, version < EPIC_9_6_1) \
	  _(impl)(array)(int64_t, value_2, 7, version >= EPIC_9_6_1 && version < EPIC_9_6_7) \
	  _(impl)(array)(float, value_2, 7, version >= EPIC_9_6_7)
CREATE_STRUCT(TS_ITEM_RANDOM_STATS);
#undef TS_ITEM_RANDOM_STATS_DEF

#define TS_ITEM_ELEMENTAL_EFFECT_DEF(_) \
	_(def)(simple) (int16_t, type) \
	_(def)(simple) (int32_t, remain_time) \
	_(def)(simple) (int32_t, attack_point) \
	_(def)(simple) (int32_t, magic_point) \
    \
	_(impl)(simple) (uint8_t, type, version < EPIC_9_8_1) \
	_(impl)(simple) (int32_t, remain_time, version < EPIC_9_8_1) \
	_(impl)(simple) (int32_t, attack_point, version < EPIC_9_8_1) \
	_(impl)(simple) (int32_t, magic_point, version < EPIC_9_8_1) \
    \
	_(impl)(simple) (int16_t, type, version >= EPIC_9_8_1) \
	_(impl)(simple) (int16_t, attack_point, version >= EPIC_9_8_1) \
	_(impl)(simple) (int16_t, magic_point, version >= EPIC_9_8_1) \
	_(impl)(simple) (int32_t, remain_time, version >= EPIC_9_8_1)
CREATE_STRUCT(TS_ITEM_ELEMENTAL_EFFECT);
#undef TS_ITEM_ELEMENTAL_EFFECT_DEF

#define TS_ITEM_BASE_INFO_DEF(_, base_info2_condition, base_info3_condition, base_info4_condition) \
	_(def)(simple)(int64_t, count) \
	_(def)(simple) (uint32_t, endurance) \
	_(def)(simple) (uint32_t, flag) \
	_(def)(simple) (int32_t, remain_time) \
	_(def)(simple) (int32_t, ethereal_durability) \
	_(def)(simple) (int32_t, appearance_code) \
	_(def)(simple) (uint32_t, summon_code) \
	_(def)(simple) (uint32_t, item_effect_id) \
    \
	_(simple) (ar_handle_t, handle) \
	_(simple) (int32_t, code) \
	_(simple) (int64_t, uid) \
	_(impl)(simple)(int32_t, count, version < EPIC_4_1_1) \
	_(impl)(simple)(int64_t, count, version >= EPIC_4_1_1) \
	_(impl)(simple) (uint32_t, flag, version >= EPIC_9_8_1) \
	_(impl)(simple) (int32_t, remain_time, version >= EPIC_9_8_1) \
	_(impl)(simple) (int32_t, ethereal_durability, version >= EPIC_6_3 && version < EPIC_9_8_1) \
	_(impl)(simple)(uint16_t, endurance, version < EPIC_4_1) \
	_(impl)(simple)(uint32_t, endurance, version >= EPIC_4_1 && version < EPIC_9_8_1) \
	_(simple) (uint8_t, enhance) \
	_(simple) (uint8_t, level) \
	_(simple) (uint16_t, enhance_chance, version >= EPIC_9_2) \
	_(simple) (uint16_t, luciad_power_level, version >= EPIC_9_8_1) \
	_(impl)(simple) (int32_t, appearance_code, version >= EPIC_9_8_1) \
	_(impl)(simple) (uint32_t, item_effect_id, version >= EPIC_9_8_1, 0) \
	_(impl)(simple) (int32_t, ethereal_durability, version >= EPIC_9_8_1) \
	_(impl)(simple) (uint32_t, endurance, version >= EPIC_9_8_1) \
	_(impl)(simple) (uint16_t, summon_code, version >= EPIC_9_8_1) \
	_(impl)(simple) (uint32_t, flag, version < EPIC_9_8_1) \
	_(simple) (TS_ITEM_SOCKETS, sockets, (version < EPIC_9_8_1 || (base_info2_condition))) \
	_(simple) (TS_ITEM_AWAKEN_OPTION, awaken_option, version >= EPIC_8_1 && (version < EPIC_9_8_1 || (base_info3_condition))) \
	_(simple) (TS_ITEM_RANDOM_STATS, random_stats, version >= EPIC_8_2 && (version < EPIC_9_8_1 || (base_info3_condition))) \
	_(impl)(simple) (int32_t, remain_time, version < EPIC_9_8_1) \
	_(simple) (TS_ITEM_ELEMENTAL_EFFECT, elemental_effect, version >= EPIC_6_1 && (version < EPIC_9_8_1 || (base_info4_condition))) \
	_(impl)(simple) (int32_t, appearance_code, version >= EPIC_7_4 && version < EPIC_9_8_1) \
	_(impl)(simple) (uint32_t, summon_code, version >= EPIC_8_2 && version < EPIC_9_8_1) \
	_(impl)(simple) (uint32_t, item_effect_id, version >= EPIC_9_6_1 && version < EPIC_9_8_1, 0)

// For BOOTH packets
#define TS_ITEM_FIXED_BASE_INFO_DEF(_) \
	TS_ITEM_BASE_INFO_DEF(_, true, true, true)
CREATE_STRUCT(TS_ITEM_FIXED_BASE_INFO);
#undef TS_ITEM_FIXED_BASE_INFO_DEF

#define TS_ITEM_FIXED_INFO_DEF(_) \
	_(def)(simple) (int16_t , wear_position) \
	_(def)(simple) (ar_handle_t, own_summon_handle) \
	_(def)(simple) (int32_t , index) \
    \
	_(simple) (int16_t , type, version >= EPIC_9_8_1) \
	_(impl)(simple) (int16_t , wear_position, version >= EPIC_9_8_1) \
	_(impl)(simple) (int16_t , index, version >= EPIC_9_8_1) \
	_(impl)(simple) (ar_handle_t, own_summon_handle, version >= EPIC_9_8_1) \
	TS_ITEM_BASE_INFO_DEF(_, true, true, true)
CREATE_STRUCT(TS_ITEM_FIXED_INFO);
#undef TS_ITEM_FIXED_INFO_DEF

#define TS_ITEM_DYNAMIC_INFO_DEF(_, use_fixed_size) \
	_(def)(simple) (int16_t , wear_position) \
	_(def)(simple) (ar_handle_t, own_summon_handle) \
	_(def)(simple) (int32_t , index) \
    \
	_(simple) (int16_t , type, version >= EPIC_9_8_1) \
	_(impl)(simple) (int16_t , wear_position, version >= EPIC_9_8_1) \
	_(impl)(simple) (int16_t , index, version >= EPIC_9_8_1) \
	_(impl)(simple) (ar_handle_t, own_summon_handle, version >= EPIC_9_8_1) \
	TS_ITEM_BASE_INFO_DEF(_, use_fixed_size || type >= 2, use_fixed_size || type >= 3, use_fixed_size || type >= 4) \
	_(impl)(simple) (int16_t , wear_position, version < EPIC_9_8_1) \
	_(impl)(simple) (ar_handle_t, own_summon_handle, version < EPIC_9_8_1) \
	_(impl)(simple) (int32_t , index, version >= EPIC_4_1_1 && version < EPIC_9_8_1)

// For TS_TRADE
#define TS_ITEM_TRADE_INFO_DEF(_) \
	TS_ITEM_DYNAMIC_INFO_DEF(_, true)
CREATE_STRUCT(TS_ITEM_TRADE_INFO);
#undef TS_ITEM_TRADE_INFO_DEF

// For TS_SC_INVENTORY
#define TS_ITEM_INFO_DEF(_) \
	TS_ITEM_DYNAMIC_INFO_DEF(_, false)
CREATE_STRUCT(TS_ITEM_INFO);
#undef TS_ITEM_INFO_DEF

#define TS_SC_INVENTORY_DEF(_) \
	_(count) (uint16_t, items) \
	_(dynarray) (TS_ITEM_INFO, items)

#define TS_SC_INVENTORY_ID(X) \
	X(207, version < EPIC_9_6_3) \
	X(1207, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_INVENTORY, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_INVENTORY_DEF

