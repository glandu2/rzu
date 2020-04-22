#pragma once

#include "Core/Timer.h"
#include "NetSession/AutoClientSession.h"
#include "NetSession/SessionServer.h"
#include "NetSession/StartableObject.h"
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "GameClient/TS_CS_LOGOUT.h"
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
#include "GameClient/TS_SC_LEAVE.h"
#include "GameClient/TS_SC_LEVEL_UPDATE.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_MOVE.h"
#include "GameClient/TS_SC_QUEST_LIST.h"
#include "GameClient/TS_SC_REMAIN_TITLE_TIME.h"
#include "GameClient/TS_SC_REMOVE_PET_INFO.h"
#include "GameClient/TS_SC_REMOVE_SUMMON_INFO.h"
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

#include "GameClient/TS_SC_PROPERTY.h"

class GameClientSession;
struct TS_CS_SET_PROPERTY;
struct TS_SC_WARP;
struct TS_SC_CHANGE_NAME;
struct TS_SC_CHANGE_TITLE_CONDITION;
struct TS_SC_UPDATE_ITEM_COUNT;
struct TS_SC_DECOMPOSE_RESULT;
struct TS_SC_DESTROY_ITEM;
struct TS_SC_HAIR_INFO;
struct TS_SC_HIDE_EQUIP_INFO;
struct TS_SC_HPMP;
struct TS_SC_REGEN_HPMP;
struct TS_SC_SKIN_INFO;

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

class ConnectionToServer : public AutoClientSession {
	DECLARE_CLASS(ConnectionToServer)

public:
	ConnectionToServer(const std::string& account, const std::string& password, const std::string& playername);
	~ConnectionToServer();

	const GameData& attachClient(GameClientSession* gameClientSession);
	const GameData& detachClient();
	void updateAllPositions();

	ar_handle_t getLocalPlayerServerHandle() {
		if(gameData.localPlayer.loginResult)
			return gameData.localPlayer.loginResult->handle;
		else
			return ar_handle_t{};
	}

	void onClientPacketReceived(const TS_CS_LOGOUT& packet);
	void onClientPacketReceived(const TS_CS_SET_PROPERTY& packet);
	template<class Packet> void onClientPacketReceived(const Packet& packet) {
		if(isConnected()) {
			sendPacket(packet);
		}
	}

protected:
	virtual void onConnected(EventTag<AutoClientSession>) override final;
	virtual void onPacketReceived(const TS_MESSAGE* packet, EventTag<AutoClientSession>) override final;
	virtual void onDisconnected(EventTag<AutoClientSession>) override final;

private:
	// Packets for creatures
	void onEnter(const TS_SC_ENTER* packet);
	void onLeave(const TS_SC_LEAVE* packet);
	void onProperty(const TS_SC_PROPERTY* packet);
	void onSkillList(const TS_SC_SKILL_LIST* packet);
	void onAddedSkillList(const TS_SC_ADDED_SKILL_LIST* packet);
	void onWearInfo(const TS_SC_WEAR_INFO* packet);
	void onItemWearInfo(const TS_SC_ITEM_WEAR_INFO* packet);
	void onExpUpdate(const TS_SC_EXP_UPDATE* packet);
	void onDetectRangeUpdate(const TS_SC_DETECT_RANGE_UPDATE* packet);
	void onSkillcardInfo(const TS_SC_SKILLCARD_INFO* packet);
	void onState(const TS_SC_STATE* packet);
	void onMove(const TS_SC_MOVE* packet);

	void onAura(const TS_SC_AURA* packet);
	void onSetMainTitle(const TS_SC_SET_MAIN_TITLE* packet);
	void onSkill(const TS_SC_SKILL* packet);
	void onStatInfo(const TS_SC_STAT_INFO* packet);
	void onSummonNotice(const TS_SC_UNSUMMON_NOTICE* packet);

	// Packets for local player
	void onLoginResult(const TS_SC_LOGIN_RESULT* packet);
	void onChat(const TS_SC_CHAT* packet);
	void onAddSummonInfo(const TS_SC_ADD_SUMMON_INFO* packet);
	void onRemoveSummonInfo(const TS_SC_REMOVE_SUMMON_INFO* packet);
	void onAddPetInfo(const TS_SC_ADD_PET_INFO* packet);
	void onRemovePetInfo(const TS_SC_REMOVE_PET_INFO* packet);
	void onInventory(const TS_SC_INVENTORY* packet);
	void onEquipSummon(const TS_EQUIP_SUMMON* packet);
	void onGoldUpdate(const TS_SC_GOLD_UPDATE* packet);
	void onBeltSlotInfo(const TS_SC_BELT_SLOT_INFO* packet);
	void onBattleArenaPenaltyInfo(const TS_SC_BATTLE_ARENA_PENALTY_INFO* packet);

	// Added functions
	void onQuestList(const TS_SC_QUEST_LIST* packet);
	void onTitleList(const TS_SC_TITLE_LIST* packet);
	void onTitleConditionList(const TS_SC_TITLE_CONDITION_LIST* packet);
	void onRemainTitleTime(const TS_SC_REMAIN_TITLE_TIME* packet);
	void onSetSubTitle(const TS_SC_SET_SUB_TITLE* packet);

	// Packets that update info in previous packets
	// Added functions
	// For creatures
	void onLevelUpdate(const TS_SC_LEVEL_UPDATE* packet);
	void onEnergy(const TS_SC_ENERGY* packet);
	void onHpMp(const TS_SC_HPMP* packet);
	void onRegenHpMp(const TS_SC_REGEN_HPMP* packet);
	void onChangeName(const TS_SC_CHANGE_NAME* packet);
	void onHairInfo(const TS_SC_HAIR_INFO* packet);
	void onSkinInfo(const TS_SC_SKIN_INFO* packet);
	void onHideEquipInfo(const TS_SC_HIDE_EQUIP_INFO* packet);
	void onStatusChange(const TS_SC_STATUS_CHANGE* packet);

	// For local player
	void onWarp(const TS_SC_WARP* packet);
	void onChangeTitleCondition(const TS_SC_CHANGE_TITLE_CONDITION* packet);
	void onUpdateItemCount(const TS_SC_UPDATE_ITEM_COUNT* packet);
	// void onDecompose(const TS_SC_DECOMPOSE_RESULT* packet) {}
	void onDestroyItem(const TS_SC_DESTROY_ITEM* packet);
	// properties that must update other packets values like hp mp exp level
	// erase item ?
	// TS_SC_ITEM_DROP_INFO ?

	CreatureData* getCreatureData(ar_handle_t creatureHandle);
	void updateCreaturePosition(CreatureData& data, float& x, float& y);

private:
	GameClientSession* gameClientSession = nullptr;

	GameData gameData;
	Timer<ConnectionToServer> moveUpdateTimer;
};

