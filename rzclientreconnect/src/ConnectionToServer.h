#pragma once

#include "Core/Timer.h"
#include "GameData.h"
#include "NetSession/AutoClientSession.h"
#include "NetSession/SessionServer.h"
#include "NetSession/StartableObject.h"
#include <memory>

class ConnectionToClient;
struct TS_CS_LOGOUT;
struct TS_CS_SET_PROPERTY;
struct TS_EQUIP_SUMMON;
struct TS_SC_ADDED_SKILL_LIST;
struct TS_SC_ADD_PET_INFO;
struct TS_SC_ADD_SUMMON_INFO;
struct TS_SC_AURA;
struct TS_SC_BATTLE_ARENA_PENALTY_INFO;
struct TS_SC_BELT_SLOT_INFO;
struct TS_SC_CHANGE_NAME;
struct TS_SC_CHANGE_TITLE_CONDITION;
struct TS_SC_CHAT;
struct TS_SC_DECOMPOSE_RESULT;
struct TS_SC_DESTROY_ITEM;
struct TS_SC_DETECT_RANGE_UPDATE;
struct TS_SC_ENERGY;
struct TS_SC_ENTER;
struct TS_SC_EXP_UPDATE;
struct TS_SC_GOLD_UPDATE;
struct TS_SC_HAIR_INFO;
struct TS_SC_HIDE_EQUIP_INFO;
struct TS_SC_HPMP;
struct TS_SC_INVENTORY;
struct TS_SC_ITEM_WEAR_INFO;
struct TS_SC_LEAVE;
struct TS_SC_LEVEL_UPDATE;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_MOVE;
struct TS_SC_PROPERTY;
struct TS_SC_QUEST_LIST;
struct TS_SC_REGEN_HPMP;
struct TS_SC_REMAIN_TITLE_TIME;
struct TS_SC_REMOVE_PET_INFO;
struct TS_SC_REMOVE_SUMMON_INFO;
struct TS_SC_SET_MAIN_TITLE;
struct TS_SC_SET_SUB_TITLE;
struct TS_SC_SKILL;
struct TS_SC_SKILLCARD_INFO;
struct TS_SC_SKILL_LIST;
struct TS_SC_SKIN_INFO;
struct TS_SC_STATE;
struct TS_SC_STATUS_CHANGE;
struct TS_SC_STAT_INFO;
struct TS_SC_TITLE_CONDITION_LIST;
struct TS_SC_TITLE_LIST;
struct TS_SC_UNSUMMON_NOTICE;
struct TS_SC_UPDATE_ITEM_COUNT;
struct TS_SC_WARP;
struct TS_SC_WEAR_INFO;

class ConnectionToServer : public AutoClientSession {
	DECLARE_CLASS(ConnectionToServer)

public:
	ConnectionToServer(const std::string& account, const std::string& password, const std::string& playername);
	~ConnectionToServer();

	const GameData& attachClient(ConnectionToClient* connectionToClient);
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
	ConnectionToClient* connectionToClient = nullptr;

	GameData gameData;
	Timer<ConnectionToServer> moveUpdateTimer;
};

