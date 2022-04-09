#include "ConnectionToServer.h"
#include "Core/Utils.h"
#include "GlobalConfig.h"
#include "MultiServerManager.h"
#include <charconv>
#include <math.h>
#include <stdlib.h>

#include "GameClient/TS_CS_LOGOUT.h"
#include "GameClient/TS_CS_SET_PROPERTY.h"
#include "GameClient/TS_SC_CHANGE_NAME.h"
#include "GameClient/TS_SC_CHANGE_TITLE_CONDITION.h"
#include "GameClient/TS_SC_DECOMPOSE_RESULT.h"
#include "GameClient/TS_SC_DESTROY_ITEM.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_HAIR_INFO.h"
#include "GameClient/TS_SC_HPMP.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_LEAVE.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_PROPERTY.h"
#include "GameClient/TS_SC_REGEN_HPMP.h"
#include "GameClient/TS_SC_REMOVE_PET_INFO.h"
#include "GameClient/TS_SC_REMOVE_SUMMON_INFO.h"
#include "GameClient/TS_SC_STATUS_CHANGE.h"
#include "GameClient/TS_SC_UPDATE_ITEM_COUNT.h"
#include "GameClient/TS_SC_WARP.h"

ConnectionToServer::ConnectionToServer(MultiServerManager* multiServerManager,
                                       const std::string& account,
                                       const std::string& password,
                                       const std::string& playername)
    : AutoClientSession(CONFIG_GET()->server.ip.get(),
                        CONFIG_GET()->server.port.get(),
                        account,
                        password,
                        CONFIG_GET()->server.serverName.get(),
                        playername,
                        CONFIG_GET()->generalConfig.epic.get(),
                        false,
                        CONFIG_GET()->generalConfig.delayTime.get(),
                        CONFIG_GET()->generalConfig.recoDelay.get()),
      multiServerManager(multiServerManager) {}

ConnectionToServer::~ConnectionToServer() {}

const GameData& ConnectionToServer::getGameData() {
	updateAllPositions();
	return gameData;
}

void ConnectionToServer::onClientPacketReceived(const TS_CS_LOGOUT& packet) {
	log(LL_Info, "Received client logout request, closing upstream server\n");
	sendPacket(packet);
	closeSession();
}

void ConnectionToServer::onClientPacketReceived(const TS_CS_SET_PROPERTY& packet) {
	gameData.localPlayer.creatureData.stringPropertiesByName[packet.name] = packet.string_value;
	sendPacket(packet);
}

void ConnectionToServer::onConnected(EventTag<AutoClientSession>) {
	log(LL_Info, "%s connected\n", getPlayerName().c_str());
	moveUpdateTimer.start(this, &ConnectionToServer::updateAllPositions, 5000, 5000);
}

void ConnectionToServer::onPacketReceived(const TS_MESSAGE* packet, EventTag<AutoClientSession>) {
	packet_type_id_t packetType = PacketMetadata::convertPacketIdToTypeId(
	    packet->id, SessionType::GameClient, SessionPacketOrigin::Server, packetVersion);
	switch(packetType) {
		case TS_SC_ENTER::packetID:
			packet->process(this, &ConnectionToServer::onEnter, packetVersion);
			break;
		case TS_SC_LEAVE::packetID:
			packet->process(this, &ConnectionToServer::onLeave, packetVersion);
			break;
		case TS_SC_PROPERTY::packetID:
			packet->process(this, &ConnectionToServer::onProperty, packetVersion);
			break;
		case TS_SC_SKILL_LIST::packetID:
			packet->process(this, &ConnectionToServer::onSkillList, packetVersion);
			break;
		case TS_SC_ADDED_SKILL_LIST::packetID:
			packet->process(this, &ConnectionToServer::onAddedSkillList, packetVersion);
			break;
		case TS_SC_WEAR_INFO::packetID:
			packet->process(this, &ConnectionToServer::onWearInfo, packetVersion);
			break;
		case TS_SC_ITEM_WEAR_INFO::packetID:
			packet->process(this, &ConnectionToServer::onItemWearInfo, packetVersion);
			break;
		case TS_SC_EXP_UPDATE::packetID:
			packet->process(this, &ConnectionToServer::onExpUpdate, packetVersion);
			break;
		case TS_SC_DETECT_RANGE_UPDATE::packetID:
			packet->process(this, &ConnectionToServer::onDetectRangeUpdate, packetVersion);
			break;
		case TS_SC_SKILLCARD_INFO::packetID:
			packet->process(this, &ConnectionToServer::onSkillcardInfo, packetVersion);
			break;
		case TS_SC_STATE::packetID:
			packet->process(this, &ConnectionToServer::onState, packetVersion);
			break;
		case TS_SC_MOVE::packetID:
			packet->process(this, &ConnectionToServer::onMove, packetVersion);
			break;
		case TS_SC_AURA::packetID:
			packet->process(this, &ConnectionToServer::onAura, packetVersion);
			break;
		case TS_SC_SET_MAIN_TITLE::packetID:
			packet->process(this, &ConnectionToServer::onSetMainTitle, packetVersion);
			break;
		case TS_SC_SKILL::packetID:
			packet->process(this, &ConnectionToServer::onSkill, packetVersion);
			break;
		case TS_SC_STAT_INFO::packetID:
			packet->process(this, &ConnectionToServer::onStatInfo, packetVersion);
			break;
		case TS_SC_UNSUMMON_NOTICE::packetID:
			packet->process(this, &ConnectionToServer::onSummonNotice, packetVersion);
			break;

		case TS_SC_LOGIN_RESULT::packetID:
			packet->process(this, &ConnectionToServer::onLoginResult, packetVersion);
			break;
		case TS_SC_CHAT::packetID:
			packet->process(this, &ConnectionToServer::onChat, packetVersion);
			break;
		case TS_SC_ADD_SUMMON_INFO::packetID:
			packet->process(this, &ConnectionToServer::onAddSummonInfo, packetVersion);
			break;
		case TS_SC_REMOVE_SUMMON_INFO::packetID:
			packet->process(this, &ConnectionToServer::onRemoveSummonInfo, packetVersion);
			break;
		case TS_SC_ADD_PET_INFO::packetID:
			packet->process(this, &ConnectionToServer::onAddPetInfo, packetVersion);
			break;
		case TS_SC_REMOVE_PET_INFO::packetID:
			packet->process(this, &ConnectionToServer::onRemovePetInfo, packetVersion);
			break;
		case TS_SC_INVENTORY::packetID:
			packet->process(this, &ConnectionToServer::onInventory, packetVersion);
			break;
		case TS_EQUIP_SUMMON::packetID:
			packet->process(this, &ConnectionToServer::onEquipSummon, packetVersion);
			break;
		case TS_SC_GOLD_UPDATE::packetID:
			packet->process(this, &ConnectionToServer::onGoldUpdate, packetVersion);
			break;
		case TS_SC_BELT_SLOT_INFO::packetID:
			packet->process(this, &ConnectionToServer::onBeltSlotInfo, packetVersion);
			break;
		case TS_SC_BATTLE_ARENA_PENALTY_INFO::packetID:
			packet->process(this, &ConnectionToServer::onBattleArenaPenaltyInfo, packetVersion);
			break;
		case TS_SC_QUEST_LIST::packetID:
			packet->process(this, &ConnectionToServer::onQuestList, packetVersion);
			break;
		case TS_SC_TITLE_LIST::packetID:
			packet->process(this, &ConnectionToServer::onTitleList, packetVersion);
			break;
		case TS_SC_TITLE_CONDITION_LIST::packetID:
			packet->process(this, &ConnectionToServer::onTitleConditionList, packetVersion);
			break;
		case TS_SC_REMAIN_TITLE_TIME::packetID:
			packet->process(this, &ConnectionToServer::onRemainTitleTime, packetVersion);
			break;
		case TS_SC_SET_SUB_TITLE::packetID:
			packet->process(this, &ConnectionToServer::onSetSubTitle, packetVersion);
			break;

		case TS_SC_LEVEL_UPDATE::packetID:
			packet->process(this, &ConnectionToServer::onLevelUpdate, packetVersion);
			break;
		case TS_SC_ENERGY::packetID:
			packet->process(this, &ConnectionToServer::onEnergy, packetVersion);
			break;
		case TS_SC_HPMP::packetID:
			packet->process(this, &ConnectionToServer::onHpMp, packetVersion);
			break;
		case TS_SC_REGEN_HPMP::packetID:
			packet->process(this, &ConnectionToServer::onRegenHpMp, packetVersion);
			break;
		case TS_SC_CHANGE_NAME::packetID:
			packet->process(this, &ConnectionToServer::onChangeName, packetVersion);
			break;
		case TS_SC_HAIR_INFO::packetID:
			packet->process(this, &ConnectionToServer::onHairInfo, packetVersion);
			break;
		case TS_SC_SKIN_INFO::packetID:
			packet->process(this, &ConnectionToServer::onSkinInfo, packetVersion);
			break;
		case TS_SC_HIDE_EQUIP_INFO::packetID:
			packet->process(this, &ConnectionToServer::onHideEquipInfo, packetVersion);
			break;
		case TS_SC_STATUS_CHANGE::packetID:
			packet->process(this, &ConnectionToServer::onStatusChange, packetVersion);
			break;
		case TS_SC_WARP::packetID:
			packet->process(this, &ConnectionToServer::onWarp, packetVersion);
			break;
		case TS_SC_CHANGE_TITLE_CONDITION::packetID:
			packet->process(this, &ConnectionToServer::onChangeTitleCondition, packetVersion);
			break;
		case TS_SC_UPDATE_ITEM_COUNT::packetID:
			packet->process(this, &ConnectionToServer::onUpdateItemCount, packetVersion);
			break;
		case TS_SC_DESTROY_ITEM::packetID:
			packet->process(this, &ConnectionToServer::onDestroyItem, packetVersion);
			break;
	}

	// Forward packet
	multiServerManager->onPacketFromServer(this, packet);
}

void ConnectionToServer::onEnter(const TS_SC_ENTER* packet) {
	log(LL_Info, "Enter handle 0x%x\n", packet->handle.get());
	if(packet->handle == getLocalPlayerServerHandle()) {
		log(LL_Warning,
		    "Server sent a TS_SC_ENTER packet with local player's handle, ignoring it so it won't be leaved on server "
		    "disconnection\n");
	} else {
		gameData.creaturesData[packet->handle].enterInfo = *packet;
		gameData.creaturesData[packet->handle].activeMove.reset();
	}
}

void ConnectionToServer::onLeave(const TS_SC_LEAVE* packet) {
	auto it = gameData.creaturesData.find(packet->handle);

	log(LL_Info, "Leave handle 0x%x\n", packet->handle.get());

	if(it == gameData.creaturesData.end())
		log(LL_Warning, "Leave of handle %x but is unknown, missing enter ?\n", packet->handle.get());
	else
		gameData.creaturesData.erase(it);
}

void ConnectionToServer::onProperty(const TS_SC_PROPERTY* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	if(packet->is_number) {
		creatureData->intPropertiesByName[packet->name] = packet->value;
		log(LL_Info, "Property: %s = %lld\n", packet->name.c_str(), (long long) packet->value);
	} else {
		creatureData->stringPropertiesByName[packet->name] = packet->string_value;
		log(LL_Info, "Property: %s = %s\n", packet->name.c_str(), packet->string_value.c_str());
	}
}

void ConnectionToServer::onSkillList(const TS_SC_SKILL_LIST* packet) {
	CreatureData* creatureData = getCreatureData(packet->target);
	if(packet->modification_type == 1)
		creatureData->skillsById.clear();
	for(const auto& skill : packet->skills) {
		creatureData->skillsById[skill.skill_id] = skill;
	}
}

void ConnectionToServer::onAddedSkillList(const TS_SC_ADDED_SKILL_LIST* packet) {
	CreatureData* creatureData = getCreatureData(packet->target);
	for(const auto& skill : packet->skills) {
		creatureData->addedSkillsById[skill.skill_id] = skill;
	}
}

void ConnectionToServer::onWearInfo(const TS_SC_WEAR_INFO* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	creatureData->wearInfo = *packet;
}

void ConnectionToServer::onItemWearInfo(const TS_SC_ITEM_WEAR_INFO* packet) {
	CreatureData* creatureData = getCreatureData(packet->target_handle);
	if(packet->wear_position >= 0)
		creatureData->itemWearByPosition[packet->item_handle] = *packet;
	else
		creatureData->itemWearByPosition.erase(packet->item_handle);
}

void ConnectionToServer::onExpUpdate(const TS_SC_EXP_UPDATE* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	creatureData->exp = *packet;
}

void ConnectionToServer::onDetectRangeUpdate(const TS_SC_DETECT_RANGE_UPDATE* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	creatureData->detectRange = *packet;
}

void ConnectionToServer::onState(const TS_SC_STATE* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	if(packet->state_level > 0)
		creatureData->statesByHandle[packet->state_handle] = *packet;
	else
		creatureData->statesByHandle.erase(packet->state_handle);
}

void ConnectionToServer::onAura(const TS_SC_AURA* packet) {
	CreatureData* creatureData = getCreatureData(packet->caster);
	if(packet->status)
		creatureData->aurasBySkillId[packet->skill_id] = *packet;
	else
		creatureData->aurasBySkillId.erase(packet->skill_id);
}

void ConnectionToServer::onSetMainTitle(const TS_SC_SET_MAIN_TITLE* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	creatureData->mainTitle = *packet;
}

void ConnectionToServer::onSkill(const TS_SC_SKILL* packet) {
	CreatureData* creatureData = getCreatureData(packet->caster);
	if(packet->type == ST_Casting || packet->type == ST_CastingUpdate)
		creatureData->activeSkillsBySkillId[packet->skill_id] = *packet;
	else
		creatureData->activeSkillsBySkillId.erase(packet->skill_id);
}

void ConnectionToServer::onStatInfo(const TS_SC_STAT_INFO* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	if(packet->type == SIT_Total)
		creatureData->statInfoTotal = *packet;
	else
		creatureData->statInfoByItems = *packet;
}

void ConnectionToServer::onSummonNotice(const TS_SC_UNSUMMON_NOTICE* packet) {
	CreatureData* creatureData = getCreatureData(packet->summon_handle);
	if(packet->unsummon_duration)
		creatureData->unsummonNotice = *packet;
	else
		creatureData->unsummonNotice.reset();
}

void ConnectionToServer::onLoginResult(const TS_SC_LOGIN_RESULT* packet) {
	gameData.localPlayer.loginResult = *packet;
	gameData.localPlayer.creatureData.enterInfo.handle = packet->handle;
}

void ConnectionToServer::onChat(const TS_SC_CHAT* packet) {
	// TODO: manage remove cases and do better handling of key
	if(packet->szSender[0] == '@') {
		gameData.localPlayer.chatPropertiesBySender[packet->szSender] = *packet;
		log(LL_Info, "Chat message %s: %s\n", packet->szSender.c_str(), packet->message.c_str());
	}

	if(packet->type == CHAT_PARTY_SYSTEM || packet->type == CHAT_RAID_SYSTEM) {
		auto args = Utils::stringSplit(packet->message, '|');

		if(args.size() < 1) {
			log(LL_Error, "Received by party message with no parameter\n");
			return;
		}

		if(args[0] == "INVITE") {
			if(!multiServerManager->isKnownLocalPlayer(args[1])) {
				log(LL_Info, "Received party invitation from unknown player %s, ignoring\n", args[1].c_str());
				return;
			}

			int32_t partyId;
			int32_t password;

			if(std::from_chars(args[3].c_str(), args[3].c_str() + args[3].size(), partyId).ec != std::errc{}) {
				log(LL_Error, "Received party invitation with invalid number for partyId: %s\n", args[3].c_str());
				return;
			}

			if(std::from_chars(args[4].c_str(), args[4].c_str() + args[4].size(), password).ec != std::errc{}) {
				log(LL_Error, "Received party invitation with invalid number for password: %s\n", args[4].c_str());
				return;
			}

			// Autoaccept invitation
			log(LL_Info, "Received party invitation from %s, autoaccepting\n", args[1].c_str());
			TS_CS_CHAT_REQUEST partyAcceptPacket{};
			Utils::stringFormat(partyAcceptPacket.message, "/pjoin %d %d", partyId, password);
			sendPacket(partyAcceptPacket);

			return;
		}

		if(args[0] == "MINFO" && args.size() >= 10) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[2]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}

			auto& memberInfo = partyInfo->membersByName[args[2]];
			memberInfo.handle = strtoul(args[1].c_str(), nullptr, 10);
			memberInfo.name = args[2];
			memberInfo.level = atoi(args[3].c_str());
			memberInfo.jobId = atoi(args[4].c_str());
			memberInfo.hpPercentage = atoi(args[5].c_str());
			memberInfo.mpPercentage = atoi(args[6].c_str());
			memberInfo.x = atoi(args[7].c_str());
			memberInfo.y = atoi(args[8].c_str());
			memberInfo.creature = atoi(args[9].c_str());
		} else if(args[0] == "RMINFO" && args.size() >= 5) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[2]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}

			auto& memberInfo = partyInfo->membersByName[args[2]];
			memberInfo.handle = strtoul(args[1].c_str(), nullptr, 10);
			memberInfo.name = args[2];
			memberInfo.jobId = atoi(args[3].c_str());
			memberInfo.hpPercentage = atoi(args[4].c_str());
		} else if(args[0] == "MSINFO" && args.size() >= 5) {
			uint32_t partyId = atoi(args[1].c_str());
			auto partyInfo = gameData.localPlayer.findPartyById(partyId);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}

			uint32_t playerHandle = atoi(args[2].c_str());
			if(playerHandle) {
				uint32_t mainSummonHandle = strtoul(args[3].c_str(), nullptr, 10);
				uint32_t subSummonHandle = strtoul(args[4].c_str(), nullptr, 10);

				if(mainSummonHandle || subSummonHandle) {
					auto& memberInfo = partyInfo->summonByPlayerHandle[playerHandle];
					memberInfo.playerHandle = playerHandle;
					memberInfo.mainSummonHandle = mainSummonHandle;
					memberInfo.subSummonHandle = subSummonHandle;
				} else {
					partyInfo->summonByPlayerHandle.erase(playerHandle);
				}
			}
		} else if(args[0] == "SINFO" && args.size() >= 4) {
			uint32_t partyId = atoi(args[1].c_str());
			auto partyInfo = gameData.localPlayer.findPartyById(partyId);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}

			size_t count = (args.size() - 3) / 3;
			partyInfo->summonByPlayerHandle.clear();
			for(size_t i = 0; i < count; i++) {
				uint32_t playerHandle = strtoul(args[3 + i * 3].c_str(), nullptr, 10);
				auto& memberInfo = partyInfo->summonByPlayerHandle[playerHandle];
				memberInfo.mainSummonHandle = strtoul(args[4 + i * 3].c_str(), nullptr, 10);
				memberInfo.subSummonHandle = strtoul(args[5 + i * 3].c_str(), nullptr, 10);
			}
		} else if(args[0] == "CHANGE_NAME" && args.size() >= 3) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[1]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			auto it = partyInfo->membersByName.find(args[1]);
			if(it != partyInfo->membersByName.end()) {
				LocalPlayerData::PartyInfo::MemberInfo memberInfo = it->second;
				memberInfo.name = args[2];
				partyInfo->membersByName[memberInfo.name] = memberInfo;
			}
		} else if(args[0] == "CHANGE_LEVEL" && args.size() >= 3) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[1]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			auto it = partyInfo->membersByName.find(args[1]);
			if(it != partyInfo->membersByName.end()) {
				it->second.level = atoi(args[2].c_str());
			} else {
				log(LL_Error, "Unknown party member %s\n", args[1].c_str());
			}
		} else if(args[0] == "CHANGE_JOB" && args.size() >= 3) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[1]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			auto it = partyInfo->membersByName.find(args[1]);
			if(it != partyInfo->membersByName.end()) {
				it->second.jobId = atoi(args[2].c_str());
			} else {
				log(LL_Error, "Unknown party member %s\n", args[1].c_str());
			}
		} else if(args[0] == "MODE" && args.size() >= 2) {
			auto partyInfo = gameData.localPlayer.findPartyByLocalMember();
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			partyInfo->shareMode = atoi(args[1].c_str());
		} else if(args[0] == "PINFO" && args.size() >= 8) {
			auto& partyInfo = gameData.localPlayer.findPartyByName(args[2]);
			partyInfo.partyId = atoi(args[1].c_str());
			partyInfo.partyName = args[2];
			partyInfo.leaderDisplayName = args[3];
			partyInfo.shareMode = atoi(args[4].c_str());
			partyInfo.maxLevel = atoi(args[5].c_str());
			partyInfo.minLevel = atoi(args[6].c_str());
			partyInfo.partyType = atoi(args[7].c_str());

			size_t memberNumber = (args.size() - 8) / 9;
			partyInfo.membersByName.clear();
			for(size_t i = 0; i < memberNumber; i++) {
				LocalPlayerData::PartyInfo::MemberInfo memberInfo;
				memberInfo.handle = strtoul(args[8 + i * 9].c_str(), nullptr, 10);
				memberInfo.name = args[9 + i * 9];
				memberInfo.level = atoi(args[10 + i * 9].c_str());
				memberInfo.jobId = atoi(args[11 + i * 9].c_str());
				memberInfo.hpPercentage = atoi(args[12 + i * 9].c_str());
				memberInfo.mpPercentage = atoi(args[13 + i * 9].c_str());
				memberInfo.x = atoi(args[14 + i * 9].c_str());
				memberInfo.y = atoi(args[15 + i * 9].c_str());
				memberInfo.creature = atoi(args[16 + i * 9].c_str());

				partyInfo.membersByName[memberInfo.name] = memberInfo;
			}
		} else if(args[0] == "LIST" && args.size() >= 5) {
			auto partyInfo = gameData.localPlayer.findPartyByLocalMember();
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			partyInfo->shareMode = atoi(args[2].c_str());
			partyInfo->maxLevel = atoi(args[3].c_str());
			partyInfo->minLevel = atoi(args[4].c_str());
		} else if(args[0] == "PPOS" && args.size() >= 4) {
			size_t memberNumber = (args.size() - 1) / 3;
			for(size_t i = 0; i < memberNumber; i++) {
				uint32_t handle = strtoul(args[1 + i * 3].c_str(), nullptr, 10);
				auto partyInfo = gameData.localPlayer.findPartyByHandle(handle);
				if(!partyInfo) {
					log(LL_Info, "%s: non existant party\n", packet->message.c_str());
					return;
				}

				for(auto& member : partyInfo->membersByName) {
					if(member.second.handle == handle) {
						member.second.x = atoi(args[2 + i * 3].c_str());
						member.second.y = atoi(args[3 + i * 3].c_str());
						break;
					}
				}
			}
		} else if(args[0] == "KICK" && args.size() >= 3) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[2]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}

			// If self was kicked, remove all party information
			if(getPlayerName() == args[2]) {
				gameData.localPlayer.partyInfoByName.erase(partyInfo->partyName);
			} else {
				auto it = partyInfo->membersByName.find(args[2]);
				if(it != partyInfo->membersByName.end())
					partyInfo->membersByName.erase(it);
			}
		} else if(args[0] == "LOGOUT" && args.size() >= 2) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[1]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			auto it = partyInfo->membersByName.find(args[1]);
			if(it != partyInfo->membersByName.end()) {
				auto& memberInfo = it->second;
				memberInfo.handle = 0;
				memberInfo.hpPercentage = 0;
				memberInfo.mpPercentage = 0;
				memberInfo.x = 0;
				memberInfo.y = 0;
				memberInfo.creature = 0;
			}
		} else if(args[0] == "LEAVE" && args.size() >= 2) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[1]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			// If self left, remove all party information
			if(getPlayerName() == args[2]) {
				gameData.localPlayer.partyInfoByName.erase(partyInfo->partyName);
			} else {
				auto it = partyInfo->membersByName.find(args[1]);
				if(it != partyInfo->membersByName.end())
					partyInfo->membersByName.erase(it);
			}
		} else if(args[0] == "PROMOTE" && args.size() >= 3) {
			auto partyInfo = gameData.localPlayer.findPartyByMember(args[2]);
			if(!partyInfo) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			auto it = partyInfo->membersByName.find(args[2]);
			if(it != partyInfo->membersByName.end())
				partyInfo->leaderDisplayName = it->second.name;
		} else if(args[0] == "DESTROY") {
			auto it = gameData.localPlayer.partyInfoByName.find(args[1]);
			if(it == gameData.localPlayer.partyInfoByName.end()) {
				log(LL_Info, "%s: non existant party\n", packet->message.c_str());
				return;
			}
			gameData.localPlayer.partyInfoByName.erase(it);
		}
	}
}

void ConnectionToServer::onAddSummonInfo(const TS_SC_ADD_SUMMON_INFO* packet) {
	gameData.localPlayer.summonInfosByCardHandle[packet->card_handle] = *packet;
}

void ConnectionToServer::onRemoveSummonInfo(const TS_SC_REMOVE_SUMMON_INFO* packet) {
	gameData.localPlayer.summonInfosByCardHandle.erase(packet->card_handle);
}

void ConnectionToServer::onAddPetInfo(const TS_SC_ADD_PET_INFO* packet) {
	gameData.localPlayer.petInfosByCardHandle[packet->pet_handle] = *packet;
}

void ConnectionToServer::onRemovePetInfo(const TS_SC_REMOVE_PET_INFO* packet) {
	gameData.localPlayer.petInfosByCardHandle.erase(packet->handle);
}

void ConnectionToServer::onInventory(const TS_SC_INVENTORY* packet) {
	for(auto& item : packet->items) {
		gameData.localPlayer.itemsByHandle[item.base_info.handle] = item;
	}
}

void ConnectionToServer::onSkillcardInfo(const TS_SC_SKILLCARD_INFO* packet) {
	if(packet->target_handle)
		gameData.localPlayer.skillCardInfoByItemHandle[packet->item_handle] = *packet;
	else
		gameData.localPlayer.skillCardInfoByItemHandle.erase(packet->item_handle);
}

void ConnectionToServer::onEquipSummon(const TS_EQUIP_SUMMON* packet) {
	gameData.localPlayer.equipSummon = *packet;
}

void ConnectionToServer::onGoldUpdate(const TS_SC_GOLD_UPDATE* packet) {
	gameData.localPlayer.gold = *packet;
}

void ConnectionToServer::onBeltSlotInfo(const TS_SC_BELT_SLOT_INFO* packet) {
	gameData.localPlayer.beltSlotInfo = *packet;
}

void ConnectionToServer::onBattleArenaPenaltyInfo(const TS_SC_BATTLE_ARENA_PENALTY_INFO* packet) {
	gameData.localPlayer.battleArenaPenaltyInfo = *packet;
}

void ConnectionToServer::onQuestList(const TS_SC_QUEST_LIST* packet) {
	gameData.localPlayer.questList = *packet;
}

void ConnectionToServer::onTitleList(const TS_SC_TITLE_LIST* packet) {
	gameData.localPlayer.titleList = *packet;
}

void ConnectionToServer::onTitleConditionList(const TS_SC_TITLE_CONDITION_LIST* packet) {
	gameData.localPlayer.titleConditionList = *packet;
}

void ConnectionToServer::onRemainTitleTime(const TS_SC_REMAIN_TITLE_TIME* packet) {
	gameData.localPlayer.titleRemainTime = *packet;
}

void ConnectionToServer::onSetSubTitle(const TS_SC_SET_SUB_TITLE* packet) {
	if(packet->code)
		gameData.localPlayer.subTitle = *packet;
	else
		gameData.localPlayer.subTitle.reset();
}

void ConnectionToServer::onLevelUpdate(const TS_SC_LEVEL_UPDATE* packet) {
	CreatureData* creatureData = getCreatureData(packet->handle);
	creatureData->level = *packet;
	creatureData->enterInfo.playerInfo.creatureInfo.level = packet->level;
}

void ConnectionToServer::onEnergy(const TS_SC_ENERGY* packet) {
	if(packet->handle == getLocalPlayerServerHandle()) {
		gameData.localPlayer.energy = *packet;
	} else {
		CreatureData* creatureData = getCreatureData(packet->handle);
		creatureData->enterInfo.playerInfo.energy = packet->energy;
	}
}

void ConnectionToServer::onHpMp(const TS_SC_HPMP* packet) {
	if(packet->handle == getLocalPlayerServerHandle()) {
		gameData.localPlayer.loginResult->hp = packet->hp;
		gameData.localPlayer.loginResult->max_hp = packet->max_hp;
		gameData.localPlayer.loginResult->mp = packet->mp;
		gameData.localPlayer.loginResult->max_mp = packet->max_mp;
	} else {
		CreatureData* creatureData = getCreatureData(packet->handle);
		creatureData->enterInfo.playerInfo.creatureInfo.hp = packet->hp;
		creatureData->enterInfo.playerInfo.creatureInfo.max_hp = packet->max_hp;
		creatureData->enterInfo.playerInfo.creatureInfo.mp = packet->mp;
		creatureData->enterInfo.playerInfo.creatureInfo.max_mp = packet->max_mp;
	}
}

void ConnectionToServer::onRegenHpMp(const TS_SC_REGEN_HPMP* packet) {
	if(packet->handle == getLocalPlayerServerHandle()) {
		gameData.localPlayer.loginResult->hp = packet->hp;
		gameData.localPlayer.loginResult->mp = packet->mp;
	} else {
		CreatureData* creatureData = getCreatureData(packet->handle);
		creatureData->enterInfo.playerInfo.creatureInfo.hp = packet->hp;
		creatureData->enterInfo.playerInfo.creatureInfo.mp = packet->mp;
	}
}

void ConnectionToServer::onChangeName(const TS_SC_CHANGE_NAME* packet) {
	if(packet->handle == getLocalPlayerServerHandle()) {
		gameData.localPlayer.loginResult->name = packet->name;
	} else {
		CreatureData* creatureData = getCreatureData(packet->handle);
		creatureData->enterInfo.playerInfo.name = packet->name;
	}
}

void ConnectionToServer::onHairInfo(const TS_SC_HAIR_INFO* packet) {
	if(packet->hPlayer == getLocalPlayerServerHandle()) {
		gameData.localPlayer.loginResult->hairId = packet->nHairID;

		// Change property only if it already exists
		auto it = gameData.localPlayer.creatureData.intPropertiesByName.find("hair_id");
		if(it != gameData.localPlayer.creatureData.intPropertiesByName.end())
			it->second = packet->nHairID;

		gameData.localPlayer.creatureData.intPropertiesByName["hair_color_idx"] = packet->nHairColorIndex;
		gameData.localPlayer.creatureData.intPropertiesByName["hair_color_rgb"] = packet->nHairColorRGB;
	} else {
		CreatureData* creatureData = getCreatureData(packet->hPlayer);
		creatureData->enterInfo.playerInfo.hairId = packet->nHairID;
		creatureData->enterInfo.playerInfo.hairColorIndex = packet->nHairColorIndex;
		creatureData->enterInfo.playerInfo.hairColorRGB = packet->nHairColorRGB;
	}
}

void ConnectionToServer::onSkinInfo(const TS_SC_SKIN_INFO* packet) {
	if(packet->handle == getLocalPlayerServerHandle()) {
		gameData.localPlayer.loginResult->skin_color = packet->nSkinColor;
	} else {
		CreatureData* creatureData = getCreatureData(packet->handle);
		creatureData->skinInfo = *packet;
	}
}

void ConnectionToServer::onHideEquipInfo(const TS_SC_HIDE_EQUIP_INFO* packet) {
	if(packet->hPlayer == getLocalPlayerServerHandle()) {
		gameData.localPlayer.hideEquipInfo = *packet;

		// Change property only if it already exists
		auto it = gameData.localPlayer.creatureData.intPropertiesByName.find("hide_equip_flag");
		if(it != gameData.localPlayer.creatureData.intPropertiesByName.end())
			it->second = packet->nHideEquipFlag;
	} else {
		CreatureData* creatureData = getCreatureData(packet->hPlayer);
		creatureData->enterInfo.playerInfo.hideEquipFlag = packet->nHideEquipFlag;
	}
}

void ConnectionToServer::onStatusChange(const TS_SC_STATUS_CHANGE* packet) {
	if(packet->handle == getLocalPlayerServerHandle()) {
		gameData.localPlayer.status = *packet;
	} else {
		CreatureData* creatureData = getCreatureData(packet->handle);
		creatureData->enterInfo.playerInfo.creatureInfo.status = packet->status;
	}
}

void ConnectionToServer::onMove(const TS_SC_MOVE* packet) {
	if(packet->handle == getLocalPlayerServerHandle()) {
		updateCreaturePosition(gameData.localPlayer.creatureData,
		                       gameData.localPlayer.loginResult->x,
		                       gameData.localPlayer.loginResult->y);

		gameData.localPlayer.creatureData.activeMove = *packet;
		gameData.localPlayer.loginResult->layer = packet->tlayer;
	} else {
		CreatureData* creatureData = getCreatureData(packet->handle);

		updateCreaturePosition(*creatureData, creatureData->enterInfo.x, creatureData->enterInfo.y);

		creatureData->activeMove = *packet;
		creatureData->enterInfo.layer = packet->tlayer;
	}
}

void ConnectionToServer::onWarp(const TS_SC_WARP* packet) {
	gameData.localPlayer.loginResult->x = packet->x;
	gameData.localPlayer.loginResult->y = packet->y;
	gameData.localPlayer.loginResult->z = packet->z;
	gameData.localPlayer.loginResult->layer = packet->layer;
}

void ConnectionToServer::onChangeTitleCondition(const TS_SC_CHANGE_TITLE_CONDITION* packet) {
	for(auto& condition : gameData.localPlayer.titleConditionList->title_condition_infos) {
		if(condition.type == packet->condition_id)
			condition.count = packet->count;
	}
}

void ConnectionToServer::onUpdateItemCount(const TS_SC_UPDATE_ITEM_COUNT* packet) {
	if(packet->count)
		gameData.localPlayer.itemsByHandle[packet->item_handle].base_info.count = packet->count;
	else
		gameData.localPlayer.itemsByHandle.erase(packet->item_handle);
}

void ConnectionToServer::onDestroyItem(const TS_SC_DESTROY_ITEM* packet) {
	gameData.localPlayer.itemsByHandle.erase(packet->item_handle);
}

CreatureData* ConnectionToServer::getCreatureData(ar_handle_t creatureHandle) {
	if(creatureHandle == getLocalPlayerServerHandle()) {
		return &gameData.localPlayer.creatureData;
	} else {
		auto it = gameData.creaturesData.find(creatureHandle);
		if(it != gameData.creaturesData.end()) {
			return &it->second;
		} else {
			log(LL_Warning,
			    "Unknown creature handle %x, creating a new one with having TS_SC_ENTER\n",
			    creatureHandle.get());
			return &gameData.creaturesData[creatureHandle];
		}
	}
}

void ConnectionToServer::updateCreaturePosition(CreatureData& data, float& x, float& y) {
	std::optional<TS_SC_MOVE>& activeMove = data.activeMove;
	if(activeMove) {
		uint32_t gameTime = getGameTime().get();

		while(!activeMove->move_infos.empty()) {
			const MOVE_INFO& moveInfo = activeMove->move_infos.front();
			float dx = moveInfo.tx - x;
			float dy = moveInfo.ty - y;
			float speed = float(activeMove->speed) / 30.0f;

			// If speed is 0, then we can't move and this step will never end
			if(speed <= 0) {
				log(LL_Error, "Bad move: speed is <= 0, this step would never end ! Skipping this buggy step");

				activeMove->move_infos.erase(activeMove->move_infos.begin());
				x = moveInfo.tx;
				y = moveInfo.ty;
				continue;
			}

			// Add 0.5f for rounding
			// Use double a float might not have enough precision for 32 bits absolute integers
			uint32_t nextStepEnd = uint32_t(double(activeMove->start_time) + sqrtf(dx * dx + dy * dy) / speed + 0.5);

			log(LL_Debug,
			    "0x%x: checking move to %d, %d, delta %d, %d, start time: %u, current time: %u\n",
			    data.enterInfo.handle.get(),
			    (int) moveInfo.tx,
			    (int) moveInfo.ty,
			    (int) dx,
			    (int) dy,
			    activeMove->start_time.get(),
			    gameTime);

			if(nextStepEnd < activeMove->start_time) {
				log(LL_Error,
				    "Bad move numbers, end time %u is less than start time %u\n",
				    nextStepEnd,
				    activeMove->start_time.get());
			}

			if(nextStepEnd <= gameTime || nextStepEnd < activeMove->start_time) {
				// step already ended
				activeMove->move_infos.erase(activeMove->move_infos.begin());
				activeMove->start_time = ar_time_t(nextStepEnd);
				x = moveInfo.tx;
				y = moveInfo.ty;
			} else {
				// step not ended
				int32_t timeSinceLastUpdate = gameTime - activeMove->start_time;
				int32_t remainingTime = nextStepEnd - activeMove->start_time;

				if(remainingTime <= 0)
					break;

				float ratio = float(timeSinceLastUpdate) / remainingTime;

				activeMove->start_time = ar_time_t(gameTime);
				x += dx * ratio;
				y += dy * ratio;

				break;
			}
		}

		if(activeMove->move_infos.empty())
			activeMove.reset();

		log(LL_Debug, "0x%x: new position: %d, %d\n", data.enterInfo.handle.get(), (int) x, (int) y);
	}
}

void ConnectionToServer::updateAllPositions() {
	// Check if we are logged in
	if(!gameData.localPlayer.loginResult)
		return;

	log(LL_Debug, "Updating all positions\n");
	updateCreaturePosition(
	    gameData.localPlayer.creatureData, gameData.localPlayer.loginResult->x, gameData.localPlayer.loginResult->y);

	for(auto& creature : gameData.creaturesData) {
		updateCreaturePosition(creature.second, creature.second.enterInfo.x, creature.second.enterInfo.y);
	}
}

void ConnectionToServer::onDisconnected(EventTag<AutoClientSession>) {
	moveUpdateTimer.stop();

	multiServerManager->onServerDisconnected(this, std::move(gameData));
	gameData.clear();

	if(gameData.localPlayer.loginResult) {
		log(LL_Error, "Game data cleared but remaining data was still their old value\n");
	}
}

LocalPlayerData::PartyInfo* LocalPlayerData::findPartyByMember(const std::string& name) {
	for(auto& party : partyInfoByName) {
		if(party.second.membersByName.count(name))
			return &party.second;
	}

	return nullptr;
}

LocalPlayerData::PartyInfo* LocalPlayerData::findPartyByHandle(uint32_t handle) {
	for(auto& party : partyInfoByName) {
		for(auto& member : party.second.membersByName) {
			if(member.second.handle == handle)
				return &party.second;
		}
	}

	return nullptr;
}

LocalPlayerData::PartyInfo* LocalPlayerData::findPartyById(int partyId) {
	for(auto& party : partyInfoByName) {
		if(party.second.partyId == partyId)
			return &party.second;
	}

	return nullptr;
}
