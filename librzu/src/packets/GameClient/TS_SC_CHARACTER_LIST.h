#pragma once

#include "Packet/PacketDeclaration.h"
#include "PacketEnums.h"

// Last tested: EPIC_9_8_1

#define LOBBY_CHARACTER_INFO_DEF(_) \
	_(simple) (uint32_t, sex) \
	_(simple) (uint32_t, race) \
	_(array)  (uint32_t, model_id, 5) \
	_(simple) (uint32_t, hair_color_index, version >= EPIC_7_1) \
	_(simple) (uint32_t, hair_color_rgb, version >= EPIC_7_1) \
	_(simple) (uint32_t, hide_equip_flag, version >= EPIC_7_1) \
	_(simple) (uint32_t, texture_id, version >= EPIC_6_3) \
	_(def)(array)(uint32_t, wear_info, 32) \
	  _(impl)(array)(uint32_t, wear_info, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, wear_info, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_info, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, wear_info, 32, version >= EPIC_9_6 && version < EPIC_9_6_7) \
	  _(impl)(array)(uint32_t, wear_info, 24, version >= EPIC_9_6_7 && version < EPIC_9_7_2) \
	_(simple) (uint32_t, level) \
	_(simple) (uint32_t, job) \
	_(simple) (uint32_t, job_level) \
	_(simple) (uint32_t, exp_percentage) \
	_(simple) (uint32_t, hp) \
	_(simple) (uint32_t, mp) \
	_(simple) (uint32_t, permission) \
	_(simple) (uint8_t, is_banned) \
	_(def)(string)(name, 20) \
	  _(impl)(string)(name, 19, version < EPIC_9_6) \
	  _(impl)(string)(name, 20, version >= EPIC_9_6) \
	_(simple) (uint32_t, skin_color, version >= EPIC_4_1) \
	_(string) (szCreateTime, 30, version < EPIC_9_6_7) \
	_(string) (szDeleteTime, 30, version < EPIC_9_6_7) \
	_(impl)(array)(uint32_t, wear_info, 10, version >= EPIC_9_7_2) \
	_(def)(array)(uint32_t, wear_item_enhance_info, 32) \
	  _(impl)(array)(uint32_t, wear_item_enhance_info, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, wear_item_enhance_info, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_item_enhance_info, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, wear_item_enhance_info, 32, version >= EPIC_9_6 && version < EPIC_9_6_7) \
	  _(impl)(array)(uint8_t,  wear_item_enhance_info, 24, version >= EPIC_9_6_7 && version < EPIC_9_7_2) \
	_(def)(array)(uint32_t, wear_item_level_info, 32) \
	  _(impl)(array)(uint32_t, wear_item_level_info, 14, version < EPIC_4_1) \
	  _(impl)(array)(uint32_t, wear_item_level_info, 24, version >= EPIC_4_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_item_level_info, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, wear_item_level_info, 32, version >= EPIC_9_6 && version < EPIC_9_6_7) \
	  _(impl)(array)(uint8_t,  wear_item_level_info, 24, version >= EPIC_9_6_7 && version < EPIC_9_7_2) \
	_(def)(array)(uint8_t, wear_item_elemental_type, 32) \
	  _(impl)(array)(uint8_t, wear_item_elemental_type, 24, version >= EPIC_6_1 && version < EPIC_9_5) \
	  _(impl)(array)(uint8_t, wear_item_elemental_type, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint8_t, wear_item_elemental_type, 32, version >= EPIC_9_6 && version < EPIC_9_6_7) \
	  _(impl)(array)(uint8_t, wear_item_elemental_type, 24, version >= EPIC_9_6_7 && version < EPIC_9_7_2) \
	_(def)(array)(uint32_t, wear_appearance_code, 32) \
	  _(impl)(array)(uint32_t, wear_appearance_code, 24, version >= EPIC_7_4 && version < EPIC_9_5) \
	  _(impl)(array)(uint32_t, wear_appearance_code, 28, version >= EPIC_9_5 && version < EPIC_9_6) \
	  _(impl)(array)(uint32_t, wear_appearance_code, 32, version >= EPIC_9_6 && version < EPIC_9_6_7) \
	  _(impl)(array)(uint32_t, wear_appearance_code, 24, version >= EPIC_9_6_7 && version < EPIC_9_7_2) \
	_(array)(uint8_t, wear_item_type, 10, version >= EPIC_9_7_2) \

CREATE_STRUCT(LOBBY_CHARACTER_INFO);
#undef LOBBY_CHARACTER_INFO_DEF // struct is 304 bytes long in epic2

#define TS_SC_CHARACTER_LIST_DEF(_) \
	_(simple)   (uint32_t, current_server_time) \
	_(simple)   (uint16_t, last_character_idx, version >= EPIC_4_1, 0) \
	_(count)    (uint16_t, characters) \
	_(dynarray) (LOBBY_CHARACTER_INFO, characters)

#define TS_SC_CHARACTER_LIST_ID(X) \
	X(2004, version < EPIC_9_6_3) \
	X(2404, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_CHARACTER_LIST, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_CHARACTER_LIST_DEF

