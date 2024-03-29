#pragma once

#include "Packet/EncodedInt.h"
#include "Packet/EncodingRandomized.h"
#include "Packet/EncodingScrambled.h"
#include "Packet/PacketDeclaration.h"

#include "TS_SC_STATUS_CHANGE.h"

// Last tested: EPIC_9_8_1

enum TS_SC_ENTER__OBJ_TYPE : uint8_t {
	EOT_Player = 0,
	EOT_NPC = 1,
	EOT_Item = 2,
	EOT_Monster = 3,
	EOT_Summon = 4,
	EOT_Skill = 5,
	EOT_FieldProp = 6,
	EOT_Pet = 7
};

enum TS_SC_ENTER__TYPE : uint8_t {
	ET_Player = 0,
	ET_NPC = 1,
	ET_StaticObject = 2,
};

#define TS_SC_ENTER__ITEM_PICK_UP_ORDER_DEF(_) \
	_(simple) (ar_time_t, drop_time) \
	_(array)  (ar_handle_t, hPlayer, 3) \
	_(array)  (int32_t, nPartyID, 3) \
	_(simple) (uint32_t, unknown, version >= EPIC_9_6_7)
CREATE_STRUCT(TS_SC_ENTER__ITEM_PICK_UP_ORDER);
#undef TS_SC_ENTER__ITEM_PICK_UP_ORDER_DEF

#define TS_SC_ENTER__ITEM_INFO_DEF(_) \
	_(simple) (EncodedInt<EncodingRandomized>, code) \
	_(def)(simple) (uint64_t, count) \
	_(impl)(simple)(uint64_t, count, version >= EPIC_4_1) \
	_(impl)(simple)(uint32_t, count, version < EPIC_4_1) \
	_(simple) (TS_SC_ENTER__ITEM_PICK_UP_ORDER, pick_up_order)
CREATE_STRUCT(TS_SC_ENTER__ITEM_INFO);
#undef TS_SC_ENTER__ITEM_INFO_DEF

#define TS_SC_ENTER__SKILL_INFO_DEF(_) \
	_(simple) (ar_handle_t, casterHandle, version >= EPIC_4_1) \
	_(simple) (ar_time_t, startTime) \
	_(simple) (uint32_t, skillId)
CREATE_STRUCT(TS_SC_ENTER__SKILL_INFO);
#undef TS_SC_ENTER__SKILL_INFO_DEF

#define TS_SC_ENTER__FIELD_PROP_INFO_DEF(_) \
	_(simple) (uint32_t, prop_id) \
	_(simple) (float, fZOffset) \
	_(simple) (float, fRotateX) \
	_(simple) (float, fRotateY) \
	_(simple) (float, fRotateZ) \
	_(simple) (float, fScaleX) /* in epic < 4, a single scale for all axis */ \
	_(simple) (float, fScaleY, version >= EPIC_4_1) \
	_(simple) (float, fScaleZ, version >= EPIC_4_1) \
	_(simple) (bool, bLockHeight, version >= EPIC_4_1) \
	_(simple) (float, fLockHeight, version >= EPIC_4_1)
CREATE_STRUCT(TS_SC_ENTER__FIELD_PROP_INFO);
#undef TS_SC_ENTER__FIELD_PROP_INFO_DEF

#define TS_SC_ENTER__CREATURE_INFO_DEF(_) \
	_(simple) (TS_CREATURE_STATUS, status) \
	_(simple) (float, face_direction) \
	_(simple) (int32_t, hp) \
	_(simple) (int32_t, max_hp) \
	_(simple) (int32_t, mp) \
	_(simple) (int32_t, max_mp) \
	_(simple) (int32_t, level) \
	_(simple) (uint8_t, race, version < EPIC_9_6_7) \
	_(simple) (uint32_t, skin_color, version >= EPIC_4_1 && version < EPIC_9_6_7) \
	_(simple) (bool, is_first_enter, version < EPIC_9_6_7) \
	_(simple) (int32_t, energy, version >= EPIC_4_1 && version < EPIC_9_6_7)
CREATE_STRUCT(TS_SC_ENTER__CREATURE_INFO);
#undef TS_SC_ENTER__CREATURE_INFO_DEF

#define TS_SC_ENTER__MONSTER_INFO_DEF(_) \
	_(simple) (TS_SC_ENTER__CREATURE_INFO, creatureInfo) \
	_(simple) (uint32_t, unknown, version >= EPIC_9_6_7) \
	_(simple) (EncodedInt<EncodingScrambled>, monster_id) \
	_(simple) (bool, is_tamed)
CREATE_STRUCT(TS_SC_ENTER__MONSTER_INFO);
#undef TS_SC_ENTER__MONSTER_INFO_DEF

#define TS_SC_ENTER__SUMMON_INFO_DEF(_) \
	_(simple) (TS_SC_ENTER__CREATURE_INFO, creatureInfo) \
	_(simple) (ar_handle_t, master_handle) \
	_(simple) (EncodedInt<EncodingRandomized>, summon_code) \
	_(def)(string)(name, 20) \
	  _(impl)(string)(name, 19, version >= EPIC_3 && version < EPIC_9_6) \
	  _(impl)(string)(name, 20, version >= EPIC_9_6) \
	_(simple) (uint8_t, enhance, version >= EPIC_7_1)
CREATE_STRUCT(TS_SC_ENTER__SUMMON_INFO);
#undef TS_SC_ENTER__SUMMON_INFO_DEF

#define TS_SC_ENTER__NPC_INFO_DEF(_) \
	_(simple) (TS_SC_ENTER__CREATURE_INFO, creatureInfo, version < EPIC_9_6_7) \
	_(simple) (TS_CREATURE_STATUS, status, version >= EPIC_9_6_7) \
	_(simple) (float, face_direction, version >= EPIC_9_6_7) \
	_(simple) (EncodedInt<EncodingRandomized>, npc_id)
CREATE_STRUCT(TS_SC_ENTER__NPC_INFO);
#undef TS_SC_ENTER__NPC_INFO_DEF

#define TS_SC_ENTER__PLAYER_INFO_DEF(_) \
	_(simple) (TS_SC_ENTER__CREATURE_INFO, creatureInfo) \
	_(simple) (uint32_t, race, version >= EPIC_9_6_7) \
	_(simple) (uint32_t, skin_color, version >= EPIC_9_6_7) \
	_(simple) (uint8_t, sex) \
	_(simple) (uint32_t, faceId) \
	_(simple) (uint32_t, faceTextureId, version >= EPIC_6_3) \
	_(simple) (uint32_t, hairId) \
	_(simple) (uint32_t, hairColorIndex, version >= EPIC_7_1) \
	_(simple) (uint32_t, hairColorRGB, version >= EPIC_7_1) \
	_(simple) (uint32_t, hideEquipFlag, version >= EPIC_7_1) \
	_(def)(string)(name, 20) \
	  _(impl)(string)(name, 19, version < EPIC_9_6) \
	  _(impl)(string)(name, 20, version >= EPIC_9_6) \
	_(simple) (uint16_t, job_id) \
	_(simple) (ar_handle_t, ride_handle) \
	_(simple) (uint32_t, guild_id) \
	_(simple) (uint32_t, title_code, version >= EPIC_8_1) \
	_(simple) (uint32_t, back_board, version >= EPIC_9_3) \
	_(simple) (int32_t, energy, version < EPIC_4_1 || version >= EPIC_9_6_7)
CREATE_STRUCT(TS_SC_ENTER__PLAYER_INFO);
#undef TS_SC_ENTER__PLAYER_INFO_DEF

#define TS_SC_ENTER__PET_INFO_DEF(_) \
	_(simple) (TS_SC_ENTER__CREATURE_INFO, creatureInfo, version < EPIC_9_6_7) \
	_(simple) (TS_CREATURE_STATUS, status, version >= EPIC_9_6_7) \
	_(simple) (float, face_direction, version >= EPIC_9_6_7) \
	_(simple) (int32_t, level, version >= EPIC_9_6_7) \
	_(simple) (ar_handle_t, master_handle) \
	_(simple) (EncodedInt<EncodingRandomized>, pet_code) \
	_(def)(string)(name, 20) \
	  _(impl)(string)(name, 19, version < EPIC_9_6) \
	  _(impl)(string)(name, 20, version >= EPIC_9_6)
CREATE_STRUCT(TS_SC_ENTER__PET_INFO);
#undef TS_SC_ENTER__PET_INFO_DEF

#define TS_SC_ENTER_DEF(_) \
	_(simple) (TS_SC_ENTER__TYPE, type) \
	_(def)(simple) (TS_SC_ENTER__OBJ_TYPE, objType) \
	_(impl)(simple) (TS_SC_ENTER__OBJ_TYPE, objType, version >= EPIC_9_6_7) \
	_(def)(simple) (uint8_t, layer) \
	_(impl)(simple) (uint8_t, layer, version >= EPIC_9_6_7) \
	_(simple) (uint8_t, unknown, version >= EPIC_9_6_7) \
	_(simple) (ar_handle_t, handle) \
	_(simple) (float, x) \
	_(simple) (float, y) \
	_(simple) (float, z) \
	_(impl)(simple) (uint8_t, layer, version < EPIC_9_6_7) \
	_(impl)(simple) (TS_SC_ENTER__OBJ_TYPE, objType, version < EPIC_9_6_7) \
	_(simple) (TS_SC_ENTER__PLAYER_INFO    , playerInfo   , objType == EOT_Player) \
	_(simple) (TS_SC_ENTER__NPC_INFO       , npcInfo      , objType == EOT_NPC) \
	_(simple) (TS_SC_ENTER__ITEM_INFO      , itemInfo     , objType == EOT_Item) \
	_(simple) (TS_SC_ENTER__MONSTER_INFO   , monsterInfo  , objType == EOT_Monster) \
	_(simple) (TS_SC_ENTER__SUMMON_INFO    , summonInfo   , objType == EOT_Summon) \
	_(simple) (TS_SC_ENTER__SKILL_INFO     , skillInfo    , objType == EOT_Skill) \
	_(simple) (TS_SC_ENTER__FIELD_PROP_INFO, fieldPropInfo, objType == EOT_FieldProp) \
	_(simple) (TS_SC_ENTER__PET_INFO       , petInfo      , objType == EOT_Pet)

#define TS_SC_ENTER_ID(X) \
	X(3, version < EPIC_9_6_3) \
	X(1003, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_ENTER, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_ENTER_DEF

