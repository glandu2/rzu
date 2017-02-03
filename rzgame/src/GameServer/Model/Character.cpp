#include "Character.h"
#include "../../GlobalConfig.h"
#include "../Database/DB_Character.h"
#include "../ReferenceData/ReferenceDataMgr.h"
#include "Core/Utils.h"
#include "../ClientSession.h"
#include "../TimeManager.h"

#include "GameClient/TS_SC_STAT_INFO.h"
#include "GameClient/TS_EQUIP_SUMMON.h"
#include "GameClient/TS_SC_WEAR_INFO.h"
#include "GameClient/TS_SC_GOLD_UPDATE.h"
#include "GameClient/TS_SC_LEVEL_UPDATE.h"
#include "GameClient/TS_SC_EXP_UPDATE.h"
#include "GameClient/TS_SC_ADDED_SKILL_LIST.h"
#include "GameClient/TS_SC_DETECT_RANGE_UPDATE.h"
#include "GameClient/TS_SC_BELT_SLOT_INFO.h"
#include "GameClient/TS_SC_STATUS_CHANGE.h"
#include "GameClient/TS_SC_QUEST_LIST.h"
#include "GameClient/TS_SC_CHANGE_LOCATION.h"
#include "GameClient/TS_SC_WEATHER_INFO.h"
#include "GameClient/TS_SC_COMMERCIAL_STORAGE_INFO.h"

namespace GameServer {

Character::Character(ClientSession* session, game_sid_t sid, const std::string& account, DB_Character *databaseData)
	: statBase(), statBuffs(), inventory(session, this)
{
	this->account = account;
	this->session = session;

	this->sid = sid;
	this->name = databaseData->name;
	this->x = databaseData->x;
	this->y = databaseData->y;
	this->z = databaseData->z;
	this->layer = databaseData->layer;
	this->face_direction = 0;
	this->hp = databaseData->hp;
	this->mp = databaseData->mp;
	this->maxHp = databaseData->hp;
	this->maxMp = databaseData->mp;
	this->level = databaseData->lv;
	this->jobLevel = databaseData->jlv;
	this->exp = databaseData->exp;
	this->jp = databaseData->jp;
	this->sex = databaseData->sex;
	this->race = databaseData->race;
	this->skinColor = databaseData->skin_color;
	this->baseModel.hairId = databaseData->model[0];
	this->baseModel.faceId = databaseData->model[1];
	this->baseModel.armorId = databaseData->model[2];
	this->baseModel.glovesId = databaseData->model[3];
	this->baseModel.bootsId = databaseData->model[4];
	this->job = databaseData->job;
	this->stamina = databaseData->stamina;
	this->chaos = databaseData->chaos;
	this->tp = databaseData->talent_point;
	this->huntaholicPoint = databaseData->huntaholic_point;
	this->huntaholicEntries = databaseData->huntaholic_enter_count;
	this->alias = databaseData->alias;
	this->arena_point = databaseData->arena_point;
	this->etherealStone = databaseData->ethereal_stone_durability;
	this->dkCount = databaseData->dkc;
	this->pkCount = databaseData->pkc;
	this->immoral = databaseData->immoral_point;
	this->permission = databaseData->permission;
	this->channel = 1;
	this->status = 0;
	this->staminaRegen = 30;
	this->clientInfo = databaseData->client_info;
	this->quickSlot = databaseData->flag_list;
	this->gold = databaseData->gold;

	static_assert(sizeof(prevJobId) == sizeof(databaseData->jobs), "Previous jobs array size mismatch");
	static_assert(sizeof(prevJobLevel) == sizeof(databaseData->jlvs), "Previous jobs level array size mismatch");
	memcpy(this->prevJobId, databaseData->jobs, sizeof(this->prevJobId));
	memcpy(this->prevJobLevel, databaseData->jlvs, sizeof(this->prevJobLevel));

	updateStats();
}

void Character::updateStats() {
	const JobResource* jobResource = ReferenceDataMgr::get()->getJob(job);
	if(!jobResource) {
		log(LL_Error, "Unknown job id: %d, can't compute stats\n", job);
		return;
	}
	const StatResource* statResource = ReferenceDataMgr::get()->getStat(jobResource->stat_id);
	if(!statResource) {
		log(LL_Error, "Unknown stat id: %d, can't compute stats\n", jobResource->stat_id);
		return;
	}

	StatBase stats(statResource);

	// + items, title, passive skill, jlv, buff
	// * items, title, passive skill, jlv, buff
	int32_t jobs[Utils_countOf(prevJobId)+1];
	int32_t jobLevels[Utils_countOf(prevJobId)+1];
	size_t i;
	for(i = 0; i < Utils_countOf(prevJobId); i++) {
		if(prevJobId[i] == 0 || prevJobLevel[i] == 0)
			break;
		jobs[i] = prevJobId[i];
		jobLevels[i] = prevJobLevel[i];
	}
	jobs[i] = job;
	jobLevels[i] = jobLevel;

	ReferenceDataMgr::get()->applyJobLevelBonus(jobs, jobLevels, i+1, &stats);

	stats.nAttackPointRight = level;
	stats.nAttackPointLeft = 0;
	stats.nAccuracyRight = level;
	stats.nAccuracyLeft = stats.nAccuracyRight;
	stats.nMagicPoint = level;
	stats.nDefence = level;
	stats.nAvoid = level;
	stats.nAttackSpeed = 100;
	stats.nMagicAccuracy = level;
	stats.nMagicDefence = level;
	stats.nMagicAvoid = level;
	stats.nMoveSpeed = 120;
	stats.nHPRegenPercentage = 5;
	stats.nMPRegenPercentage = 5;
	stats.nBlockChance = 0;
	stats.nBlockDefence = 0;
	stats.nCritical = 3;
	stats.nCastingSpeed = 100;
	stats.nHPRegenPoint = 48 + 2 * level;
	stats.nMPRegenPoint = 48 + 2 * level;
	stats.nCriticalPower = 80;
	stats.nCoolTimeSpeed = 100;
	stats.nMaxWeight = 10 * level;
	stats.nItemChance = 0;
	stats.nPerfectBlock = 20;
	stats.nAttackRange = 50;
	stats.nMagicalDefIgnore = 0;
	stats.nMagicalDefIgnoreRatio = 0;
	stats.nPhysicalDefIgnore = 0;
	stats.nPhysicalDefIgnoreRatio = 0;
	stats.nMagicalPenetration = 0;
	stats.nMagicalPenetrationRatio = 0;
	stats.nPhysicalPenetration = 0;
	stats.nPhysicalPenetrationRatio = 0;
	stats.maxHp = 20 * level;
	stats.maxMp = 20 * level;
	stats.maxStamina = 500000;
	stats.maxChaos = 500;

	statBase = stats;

	if(true) {
		stats.nAttackPointRight += 2.8f * statBase.strength;
	} else {
		stats.nAttackPointRight += 1.2f * statBase.agility + 2.2f * statBase.dexterity;
	}
	stats.nAccuracyRight += 0.5f * statBase.dexterity;
	stats.nAccuracyLeft += statBase.nAccuracyRight;
	stats.nMagicPoint += 2 * statBase.intelligence;
	stats.nDefence += 1.6f * statBase.vitality;
	stats.nAvoid += 0.5f * statBase.agility;
	stats.nAttackSpeed += 0.1f * statBase.agility;
	stats.nMagicAccuracy += 0.4f * statBase.wisdom;
	stats.nMagicDefence += 2 * statBase.wisdom;
	stats.nMagicAvoid += 0.5f * statBase.wisdom;
	stats.nCritical += statBase.luck / 5;
	stats.nMPRegenPoint += 4.1f * statBase.wisdom;
	stats.nMaxWeight += 10 * statBase.strength;
	stats.nItemChance += statBase.luck / 5;
	stats.maxHp += 33 * statBase.vitality;
	stats.maxMp += 30 * statBase.intelligence;

	memset(&statBuffs, 0, sizeof(statBuffs));

	sendPacketStats();
}

void Character::sendPacketStats() {
	statBase.sendPacket(session, handle, StatBase::Base);
	statBuffs.sendPacket(session, handle, StatBase::Buff);

	session->sendProperty(handle, "max_chaos", maxChaos);
	session->sendProperty(handle, "max_stamina", maxStamina);
}

void Character::sendEquip() {
	TS_SC_WEAR_INFO wearInfo = {0};
	wearInfo.handle = handle;
	for(size_t i = 0; i < Utils_countOf(wearInfo.wear_info); i++) {
		const Item* item = inventory.getEquipedItem((Inventory::ItemWearType)i);

		if(item) {
			wearInfo.wear_info[i] = item->code;
			wearInfo.wear_item_level_info[i] = item->level;
			wearInfo.wear_item_enhance_info[i] = item->enhance;
			wearInfo.wear_item_elemental_type[i] = item->elemental_effect_type;
			wearInfo.wear_appearance_code[i] = item->appearance_code;
		} else if(i == Inventory::WEAR_ARMOR) {
			wearInfo.wear_info[i] = baseModel.armorId;
		} else if(i == Inventory::WEAR_GLOVE) {
			wearInfo.wear_info[i] = baseModel.glovesId;
		} else if(i == Inventory::WEAR_BOOTS) {
			wearInfo.wear_info[i] = baseModel.bootsId;
		}
	}
	session->sendPacket(wearInfo);
}

void Character::synchronizeWithClient() {
	sendPacketStats();

	inventory.sendInventory();

	TS_EQUIP_SUMMON equipSummon = {0};
	session->sendPacket(equipSummon);

	sendEquip();

	TS_SC_GOLD_UPDATE goldUpdate;
	goldUpdate.chaos = chaos;
	goldUpdate.gold = gold;
	session->sendPacket(goldUpdate);

	session->sendProperty(handle, "tp", tp);
	session->sendProperty(handle, "chaos", chaos);

	TS_SC_LEVEL_UPDATE levelUpdate;
	levelUpdate.handle = handle;
	levelUpdate.job_level = jobLevel;
	levelUpdate.level = level;
	session->sendPacket(levelUpdate);

	TS_SC_EXP_UPDATE expUpdate;
	expUpdate.exp = exp;
	expUpdate.handle = handle;
	expUpdate.jp = jp;
	session->sendPacket(expUpdate);

	session->sendProperty(handle, "job", job);
	session->sendProperty(handle, "job_level", jobLevel);
	session->sendProperty(handle, "job_0", prevJobId[0]);
	session->sendProperty(handle, "jlv_0", prevJobLevel[0]);
	session->sendProperty(handle, "job_1", prevJobId[1]);
	session->sendProperty(handle, "jlv_1", prevJobLevel[1]);
	session->sendProperty(handle, "job_2", prevJobId[2]);
	session->sendProperty(handle, "jlv_2", prevJobLevel[2]);

	TS_SC_ADDED_SKILL_LIST skillList;
	skillList.target = handle;
	session->sendPacket(skillList);

	TS_SC_DETECT_RANGE_UPDATE rangeUpdate;
	rangeUpdate.handle = handle;
	rangeUpdate.detect_range = 0;
	session->sendPacket(rangeUpdate);

	TS_SC_BELT_SLOT_INFO beltSlotInfo = {0};
	session->sendPacket(beltSlotInfo);

	TimeManager::sendGameTime(session);

	session->sendProperty(handle, "huntaholicpoint", huntaholicPoint);
	session->sendProperty(handle, "huntaholic_ent", huntaholicEntries);
	session->sendProperty(handle, "ap", arena_point);
	session->sendProperty(handle, "alias", alias);
	session->sendProperty(handle, "ethereal_stone", etherealStone);
	session->sendProperty(handle, "dk_count", dkCount);
	session->sendProperty(handle, "pk_count", pkCount);
	session->sendProperty(handle, "immoral", immoral);
	session->sendProperty(handle, "permission", permission);
	session->sendProperty(handle, "stamina", stamina);
	session->sendProperty(handle, "max_stamina", maxStamina);
	session->sendProperty(handle, "channel", channel);

	TS_SC_STATUS_CHANGE statusChange;
	statusChange.handle = handle;
	statusChange.status = 0;
	session->sendPacket(statusChange);

	TS_SC_QUEST_LIST questList;
	session->sendPacket(questList);

	session->sendProperty(handle, "playtime", 0);
	session->sendProperty(handle, "playtime_limit1", 1080000);
	session->sendProperty(handle, "playtime_limit2", 1800000);

	TS_SC_CHANGE_LOCATION changeLocation;
	changeLocation.prev_location_id = 0;
	changeLocation.cur_location_id = 0;
	session->sendPacket(changeLocation);

	TS_SC_WEATHER_INFO weatherInfo;
	weatherInfo.region_id = 0;
	weatherInfo.weather_id = 0;
	session->sendPacket(weatherInfo);

	TS_SC_COMMERCIAL_STORAGE_INFO commercialStorage;
	commercialStorage.new_item_count = 0;
	commercialStorage.total_item_count = 0;
	session->sendPacket(commercialStorage);

	session->sendProperty(handle, "client_info", clientInfo);
	session->sendProperty(handle, "quick_slot", clientInfo);
	session->sendProperty(handle, "current_key", clientInfo);
	session->sendProperty(handle, "saved_key", clientInfo);
	session->sendProperty(handle, "stamina_regen", staminaRegen);
}

}
