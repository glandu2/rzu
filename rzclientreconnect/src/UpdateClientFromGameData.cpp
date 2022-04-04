#include "UpdateClientFromGameData.h"
#include "Core/Utils.h"

#include "GameClient/TS_SC_CHANGE_NAME.h"
#include "GameClient/TS_SC_HPMP.h"
#include "GameClient/TS_SC_LEAVE.h"
#include "GameClient/TS_SC_PROPERTY.h"
#include "GameClient/TS_SC_REMOVE_PET_INFO.h"
#include "GameClient/TS_SC_REMOVE_SUMMON_INFO.h"
#include "GameClient/TS_SC_SKIN_INFO.h"
#include "GameClient/TS_SC_UPDATE_ITEM_COUNT.h"
#include "GameClient/TS_SC_WARP.h"

UpdateClientFromGameData::UpdateClientFromGameData(IPacketInterface* clientPacketInterface)
    : clientPacketInterface(clientPacketInterface) {}

void UpdateClientFromGameData::removePlayerData(const GameData& gameData) {
	log(LL_Info, "Removing player and items from the client\n");

	// If no login result, no data to remove
	if(!gameData.localPlayer.loginResult)
		return;

	// TODO: handle Chat packets: parties, guild, other ?

	removeCreatureData(gameData.localPlayer.creatureData, true);

	for(const auto& it : gameData.localPlayer.summonInfosByCardHandle) {
		TS_SC_REMOVE_SUMMON_INFO packet;
		packet.card_handle = it.first;
		clientPacketInterface->onPacketFromServer(packet);
	}

	for(const auto& it : gameData.localPlayer.petInfosByCardHandle) {
		TS_SC_REMOVE_PET_INFO packet;
		packet.handle = it.first;
		clientPacketInterface->onPacketFromServer(packet);
	}

	for(const auto& it : gameData.localPlayer.skillCardInfoByItemHandle) {
		TS_SC_SKILLCARD_INFO packet;
		packet.item_handle = it.first;
		packet.target_handle = ar_handle_t(0);
		clientPacketInterface->onPacketFromServer(packet);
	}

	for(const auto& it : gameData.localPlayer.itemsByHandle) {
		ar_handle_t handle = it.first;
		TS_SC_UPDATE_ITEM_COUNT deleteItemMessage;
		deleteItemMessage.item_handle = handle;
		deleteItemMessage.count = 0;
		clientPacketInterface->onPacketFromServer(deleteItemMessage);
	}

	for(auto& party : gameData.localPlayer.partyInfoByName) {
		TS_SC_CHAT packet;
		packet.szSender = "@PARTY";
		packet.type = CHAT_PARTY_SYSTEM;
		Utils::stringFormat(packet.message, "DESTROY|%s|%d|", party.second.partyName.c_str(), 0);
		clientPacketInterface->onPacketFromServer(packet);
	}

	for(const auto& it : gameData.creaturesData) {
		ar_handle_t handle = it.first;
		if(handle == gameData.localPlayer.loginResult->handle) {
			log(LL_Warning, "Skipping sending TS_SC_LEAVE with client's local player's handle\n");
			continue;
		}
		removeCreatureData(it.second, false);
	}
}

void UpdateClientFromGameData::removeCreatureData(const CreatureData& creatureData, bool isLocalPlayer) {
	if(isLocalPlayer) {
		for(const auto& skill : creatureData.activeSkillsBySkillId) {
			TS_SC_SKILL skillPacket = skill.second;
			skillPacket.type = ST_Cancel;
			clientPacketInterface->onPacketFromServer(skillPacket);
		}

		for(const auto& aura : creatureData.aurasBySkillId) {
			auto packet = aura.second;
			packet.status = 0;
			clientPacketInterface->onPacketFromServer(packet);
		}

		for(const auto& state : creatureData.statesByHandle) {
			TS_SC_STATE packet = state.second;
			packet.state_level = 0;
			clientPacketInterface->onPacketFromServer(packet);
		}

		for(const auto& it : creatureData.itemWearByPosition) {
			TS_SC_ITEM_WEAR_INFO packet = it.second;
			packet.wear_position = -1;
			clientPacketInterface->onPacketFromServer(packet);
		}
	} else {
		TS_SC_LEAVE leaveMessage;
		leaveMessage.handle = creatureData.enterInfo.handle;
		clientPacketInterface->onPacketFromServer(leaveMessage);
	}
}

void UpdateClientFromGameData::addPlayerData(const GameData& gameData) {
	log(LL_Info, "Adding player and items to the client\n");

	// If no login result, we are not yet connected
	if(!gameData.localPlayer.loginResult)
		return;

	// Send local player data
	sendLoginResultToClient(&gameData.localPlayer.loginResult.value());

	//	for(const auto& packet : gameData.localPlayer.chatPropertiesBySender)
	//		clientPacketInterface->onPacketFromServer(packet.second);

	if(!gameData.localPlayer.itemsByHandle.empty()) {
		TS_SC_INVENTORY inventoryPacket;
		auto it = gameData.localPlayer.itemsByHandle.begin();
		while(it != gameData.localPlayer.itemsByHandle.end()) {
			inventoryPacket.items.push_back(it->second);

			// If packet cannot have a additional item without going above max packet size,
			// send it and clear inventory packet's items
			if(inventoryPacket.getSize(clientPacketInterface->getPacketVersion()) +
			       it->second.getSize(clientPacketInterface->getPacketVersion()) >
			   16384) {
				clientPacketInterface->onPacketFromServer(inventoryPacket);
				inventoryPacket.items.clear();
			}
			++it;
		}
		if(!inventoryPacket.items.empty()) {
			clientPacketInterface->onPacketFromServer(inventoryPacket);
		}
	}

	for(const auto& packet : gameData.localPlayer.summonInfosByCardHandle)
		clientPacketInterface->onPacketFromServer(packet.second);
	for(const auto& packet : gameData.localPlayer.petInfosByCardHandle)
		clientPacketInterface->onPacketFromServer(packet.second);

	if(gameData.localPlayer.equipSummon)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.equipSummon.value());
	if(gameData.localPlayer.gold)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.gold.value());
	if(gameData.localPlayer.beltSlotInfo)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.beltSlotInfo.value());
	if(gameData.localPlayer.battleArenaPenaltyInfo)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.battleArenaPenaltyInfo.value());

	if(gameData.localPlayer.questList)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.questList.value());
	if(gameData.localPlayer.titleList)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.titleList.value());
	if(gameData.localPlayer.titleConditionList)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.titleConditionList.value());
	if(gameData.localPlayer.titleRemainTime)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.titleRemainTime.value());
	if(gameData.localPlayer.subTitle)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.subTitle.value());

	sendCreatureData(gameData.localPlayer.creatureData, true);

	for(const auto& packet : gameData.localPlayer.skillCardInfoByItemHandle)
		clientPacketInterface->onPacketFromServer(packet.second);

	if(gameData.localPlayer.energy)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.energy.value());
	if(gameData.localPlayer.hideEquipInfo)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.hideEquipInfo.value());

	if(gameData.localPlayer.status)
		clientPacketInterface->onPacketFromServer(gameData.localPlayer.status.value());

	for(auto& party : gameData.localPlayer.partyInfoByName) {
		sendPartyInfo(&party.second);
	}

	// Send other creatures data
	for(const auto& creature : gameData.creaturesData)
		sendCreatureData(creature.second, false);
}

void UpdateClientFromGameData::sendLoginResultToClient(const TS_SC_LOGIN_RESULT* loginResult) {
	TS_SC_WARP warpPacket;
	warpPacket.x = loginResult->x;
	warpPacket.y = loginResult->y;
	warpPacket.z = loginResult->z;
	warpPacket.layer = loginResult->layer;
	clientPacketInterface->onPacketFromServer(warpPacket);

	//	TS_SC_MOVE movePacket;
	//	movePacket.speed = INT8_MAX;
	//	movePacket.tlayer = loginResult->layer;
	//	movePacket.handle = loginResult->handle;
	//	movePacket.start_time = ar_time_t(0);
	//	movePacket.move_infos.push_back(MOVE_INFO{loginResult->x, loginResult->y});
	//	clientPacketInterface->onPacketFromServer(movePacket);

	TS_SC_HPMP hpmpPacket{};
	hpmpPacket.handle = loginResult->handle;
	hpmpPacket.need_to_display = false;  // if true and add_xx not 0, display in client as damage
	hpmpPacket.hp = loginResult->hp;
	hpmpPacket.mp = loginResult->mp;
	hpmpPacket.max_hp = loginResult->max_hp;
	hpmpPacket.max_mp = loginResult->max_mp;
	clientPacketInterface->onPacketFromServer(hpmpPacket);

	std::initializer_list<std::pair<std::string, int64_t>> properties = {
	    {"sex", loginResult->sex},
	    {"race", loginResult->race},
	    {"hairId", loginResult->hairId},
	    {"guild_id", loginResult->guild_id},
	    {"back_board", loginResult->back_board},  // TODO: not sure
	};

	for(const auto& property : properties) {
		TS_SC_PROPERTY propertyPacket;
		propertyPacket.handle = loginResult->handle;
		propertyPacket.name = property.first;
		propertyPacket.is_number = true;
		propertyPacket.value = property.second;
		clientPacketInterface->onPacketFromServer(hpmpPacket);
	}

	TS_SC_SKIN_INFO skinInfoPacket;
	skinInfoPacket.handle = loginResult->handle;
	skinInfoPacket.nSkinColor = loginResult->skin_color;
	clientPacketInterface->onPacketFromServer(skinInfoPacket);

	TS_SC_CHANGE_NAME changeNamePacket;
	changeNamePacket.handle = loginResult->handle;
	changeNamePacket.name = loginResult->name;
	clientPacketInterface->onPacketFromServer(changeNamePacket);
}

void UpdateClientFromGameData::sendPartyInfo(const LocalPlayerData::PartyInfo* party) {
	TS_SC_CHAT packet;
	packet.szSender = "@PARTY";
	packet.type = CHAT_PARTY_SYSTEM;

	for(auto& member : party->membersByName) {
		Utils::stringFormat(packet.message,
		                    "MINFO|%u|%s|%d|%d|%d|%d|%d|%d|%d|",
		                    member.second.handle,
		                    member.second.name.c_str(),
		                    member.second.level,
		                    member.second.jobId,
		                    member.second.hpPercentage,
		                    member.second.mpPercentage,
		                    member.second.x,
		                    member.second.y,
		                    member.second.creature);
		clientPacketInterface->onPacketFromServer(packet);
	}

	Utils::stringFormat(packet.message,
	                    "PINFO|%d|%s|%s|%d|%d|%d|%d|",
	                    party->partyId,
	                    party->partyName.c_str(),
	                    party->leaderDisplayName.c_str(),
	                    party->shareMode,
	                    party->maxLevel,
	                    party->minLevel,
	                    party->partyType);
	for(auto& member : party->membersByName) {
		std::string buffer;
		Utils::stringFormat(buffer,
		                    "%u|%s|%d|%d|%d|%d|%d|%d|%d|",
		                    member.second.handle,
		                    member.second.name.c_str(),
		                    member.second.level,
		                    member.second.jobId,
		                    member.second.hpPercentage,
		                    member.second.mpPercentage,
		                    member.second.x,
		                    member.second.y,
		                    member.second.creature);
		packet.message += buffer;
	}
	clientPacketInterface->onPacketFromServer(packet);

	if(!party->summonByPlayerHandle.empty()) {
		Utils::stringFormat(packet.message, "SINFO|%d|%d|", party->partyId, (int) party->summonByPlayerHandle.size());

		for(auto& summon : party->summonByPlayerHandle) {
			std::string buffer;
			Utils::stringFormat(buffer,
			                    "%u|%u|%u|",
			                    summon.second.playerHandle,
			                    summon.second.mainSummonHandle,
			                    summon.second.subSummonHandle);
			packet.message += buffer;
		}
		clientPacketInterface->onPacketFromServer(packet);
	}

	packet.message = "ARENAMODE|-1|0";
	clientPacketInterface->onPacketFromServer(packet);
}

void UpdateClientFromGameData::sendCreatureData(const CreatureData& creatureData, bool isForLocalPlayer) {
	if(!isForLocalPlayer)
		clientPacketInterface->onPacketFromServer(creatureData.enterInfo);

	if(!creatureData.skillsById.empty()) {
		TS_SC_SKILL_LIST skillListPacket;
		skillListPacket.target = creatureData.enterInfo.handle;
		// 0: UPDATE, 1: REFRESH (reset + add)
		skillListPacket.modification_type = 1;
		for(const auto& packet : creatureData.skillsById)
			skillListPacket.skills.push_back(packet.second);
		clientPacketInterface->onPacketFromServer(skillListPacket);
	}

	if(!creatureData.skillsById.empty()) {
		TS_SC_ADDED_SKILL_LIST skillListPacket;
		skillListPacket.target = creatureData.enterInfo.handle;
		for(const auto& packet : creatureData.addedSkillsById)
			skillListPacket.skills.push_back(packet.second);
		clientPacketInterface->onPacketFromServer(skillListPacket);
	}

	for(const auto& property : creatureData.stringPropertiesByName) {
		TS_SC_PROPERTY propertyMessage;
		propertyMessage.handle = creatureData.enterInfo.handle;
		propertyMessage.name = property.first;
		propertyMessage.is_number = false;
		propertyMessage.string_value = property.second;
		clientPacketInterface->onPacketFromServer(propertyMessage);
	}

	for(const auto& property : creatureData.intPropertiesByName) {
		TS_SC_PROPERTY propertyMessage;
		propertyMessage.handle = creatureData.enterInfo.handle;
		propertyMessage.name = property.first;
		propertyMessage.is_number = true;
		propertyMessage.value = property.second;
		clientPacketInterface->onPacketFromServer(propertyMessage);
	}

	if(creatureData.wearInfo)
		clientPacketInterface->onPacketFromServer(creatureData.wearInfo.value());

	for(const auto& packet : creatureData.itemWearByPosition)
		clientPacketInterface->onPacketFromServer(packet.second);

	if(creatureData.level)
		clientPacketInterface->onPacketFromServer(creatureData.level.value());
	if(creatureData.exp)
		clientPacketInterface->onPacketFromServer(creatureData.exp.value());
	if(creatureData.detectRange)
		clientPacketInterface->onPacketFromServer(creatureData.detectRange.value());

	for(const auto& packet : creatureData.statesByHandle)
		clientPacketInterface->onPacketFromServer(packet.second);

	for(const auto& packet : creatureData.aurasBySkillId)
		clientPacketInterface->onPacketFromServer(packet.second);

	if(creatureData.mainTitle)
		clientPacketInterface->onPacketFromServer(creatureData.mainTitle.value());

	for(const auto& packet : creatureData.activeSkillsBySkillId)
		clientPacketInterface->onPacketFromServer(packet.second);

	if(creatureData.statInfoTotal)
		clientPacketInterface->onPacketFromServer(creatureData.statInfoTotal.value());

	if(creatureData.statInfoByItems)
		clientPacketInterface->onPacketFromServer(creatureData.statInfoByItems.value());

	if(creatureData.unsummonNotice)
		clientPacketInterface->onPacketFromServer(creatureData.unsummonNotice.value());

	if(creatureData.skinInfo)
		clientPacketInterface->onPacketFromServer(creatureData.skinInfo.value());

	if(creatureData.activeMove)
		clientPacketInterface->onPacketFromServer(creatureData.activeMove.value());
}