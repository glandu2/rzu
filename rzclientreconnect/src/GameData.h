#pragma once

#include <optional>
#include <unordered_map>

#include "GameClient/TS_EQUIP_SUMMON.h"
#include "GameClient/TS_SC_ADDED_SKILL_LIST.h"
#include "GameClient/TS_SC_ADD_PET_INFO.h"
#include "GameClient/TS_SC_ADD_SUMMON_INFO.h"
#include "GameClient/TS_SC_AURA.h"
#include "GameClient/TS_SC_BATTLE_ARENA_PENALTY_INFO.h"
#include "GameClient/TS_SC_BELT_SLOT_INFO.h"
#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_DETECT_RANGE_UPDATE.h"
#include "GameClient/TS_SC_ENERGY.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_EXP_UPDATE.h"
#include "GameClient/TS_SC_GOLD_UPDATE.h"
#include "GameClient/TS_SC_HIDE_EQUIP_INFO.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_ITEM_WEAR_INFO.h"
#include "GameClient/TS_SC_LEVEL_UPDATE.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_MOVE.h"
#include "GameClient/TS_SC_QUEST_LIST.h"
#include "GameClient/TS_SC_REMAIN_TITLE_TIME.h"
#include "GameClient/TS_SC_SET_MAIN_TITLE.h"
#include "GameClient/TS_SC_SET_SUB_TITLE.h"
#include "GameClient/TS_SC_SKILL.h"
#include "GameClient/TS_SC_SKILLCARD_INFO.h"
#include "GameClient/TS_SC_SKILL_LIST.h"
#include "GameClient/TS_SC_SKIN_INFO.h"
#include "GameClient/TS_SC_STATE.h"
#include "GameClient/TS_SC_STATUS_CHANGE.h"
#include "GameClient/TS_SC_STAT_INFO.h"
#include "GameClient/TS_SC_TITLE_CONDITION_LIST.h"
#include "GameClient/TS_SC_TITLE_LIST.h"
#include "GameClient/TS_SC_UNSUMMON_NOTICE.h"
#include "GameClient/TS_SC_WEAR_INFO.h"

// Data associated with an handle (local player, summons, other players, monsters, pets, items)
struct CreatureData {
	TS_SC_ENTER enterInfo{};
	std::unordered_map<std::string, std::string> stringPropertiesByName;
	std::unordered_map<std::string, int64_t> intPropertiesByName;
	std::unordered_map<int32_t, TS_SKILL_INFO> skillsById;
	std::unordered_map<int32_t, TS_ADDED_SKILL_LIST> addedSkillsById;
	std::optional<TS_SC_WEAR_INFO> wearInfo{};
	std::unordered_map<ar_handle_t, TS_SC_ITEM_WEAR_INFO> itemWearByPosition;
	std::optional<TS_SC_LEVEL_UPDATE> level{};
	std::optional<TS_SC_EXP_UPDATE> exp{};
	std::optional<TS_SC_DETECT_RANGE_UPDATE> detectRange{};
	std::unordered_map<uint16_t, TS_SC_STATE> statesByHandle;

	std::unordered_map<uint16_t, TS_SC_AURA> aurasBySkillId;
	std::optional<TS_SC_SET_MAIN_TITLE> mainTitle{};
	std::unordered_map<uint16_t, TS_SC_SKILL> activeSkillsBySkillId;
	std::optional<TS_SC_STAT_INFO> statInfoTotal{};
	std::optional<TS_SC_STAT_INFO> statInfoByItems{};
	std::optional<TS_SC_UNSUMMON_NOTICE> unsummonNotice{};
	std::optional<TS_SC_SKIN_INFO> skinInfo{};

	std::optional<TS_SC_MOVE> activeMove{};
};

// Data implicitly bound to local player (no handle in messages)
struct LocalPlayerData {
	CreatureData creatureData;
	std::optional<TS_SC_LOGIN_RESULT> loginResult{};
	std::unordered_map<std::string, TS_SC_CHAT> chatPropertiesBySender;

	struct PartyInfo {
		int partyId{};
		std::string partyName;
		std::string leaderDisplayName;
		int shareMode{};
		int maxLevel{};
		int minLevel{};
		int partyType{};

		struct MemberInfo {
			uint32_t handle{};
			std::string name;
			int level{};
			int jobId{};
			int hpPercentage{};
			int mpPercentage{};
			int x{};
			int y{};
			int creature{};
		};
		std::unordered_map<std::string, MemberInfo> membersByName;

		struct SummonInfo {
			uint32_t playerHandle;
			uint32_t mainSummonHandle;
			uint32_t subSummonHandle;
		};
		std::unordered_map<uint32_t, SummonInfo> summonByPlayerHandle;
	};
	std::unordered_map<std::string, PartyInfo> partyInfoByName;

	PartyInfo& findPartyByName(const std::string& name) { return partyInfoByName[name]; }
	PartyInfo* findPartyByMember(const std::string& name);
	PartyInfo* findPartyByLocalMember() { return findPartyByMember(loginResult->name); }
	PartyInfo* findPartyByHandle(uint32_t handle);
	PartyInfo* findPartyById(int partyId);

	std::unordered_map<ar_handle_t, TS_SC_ADD_SUMMON_INFO> summonInfosByCardHandle;
	std::unordered_map<ar_handle_t, TS_SC_ADD_PET_INFO> petInfosByCardHandle;
	std::unordered_map<ar_handle_t, TS_ITEM_INFO> itemsByHandle;
	std::unordered_map<ar_handle_t, TS_SC_SKILLCARD_INFO> skillCardInfoByItemHandle;
	std::optional<TS_EQUIP_SUMMON> equipSummon{};  // set open_dialog to false
	std::optional<TS_SC_GOLD_UPDATE> gold{};
	std::optional<TS_SC_BELT_SLOT_INFO> beltSlotInfo{};
	std::optional<TS_SC_BATTLE_ARENA_PENALTY_INFO> battleArenaPenaltyInfo{};

	std::optional<TS_SC_QUEST_LIST> questList{};
	std::optional<TS_SC_TITLE_LIST> titleList{};
	std::optional<TS_SC_TITLE_CONDITION_LIST> titleConditionList{};
	std::optional<TS_SC_REMAIN_TITLE_TIME> titleRemainTime{};
	std::optional<TS_SC_SET_SUB_TITLE> subTitle{};
	std::optional<TS_SC_ENERGY> energy{};
	std::optional<TS_SC_HIDE_EQUIP_INFO> hideEquipInfo{};
	std::optional<TS_SC_STATUS_CHANGE> status{};
	// DEATHMATCH_RANKING_OPEN
};

struct GameData {
	LocalPlayerData localPlayer;
	std::unordered_map<ar_handle_t, CreatureData> creaturesData;

	void clear();
};