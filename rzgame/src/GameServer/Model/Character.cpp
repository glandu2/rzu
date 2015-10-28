#include "Character.h"
#include "../../GlobalConfig.h"
#include "../Database/DB_Character.h"
#include "../ReferenceData/ReferenceDataMgr.h"
#include "Core/Utils.h"
#include "../ClientSession.h"
#include "../TimeManager.h"

#include "GameClient/TS_SC_STAT_INFO.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_EQUIP_SUMMON.h"
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
	: statBase(), statBuffs()
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
	this->hairId = databaseData->model[0];
	this->faceId = databaseData->model[1];
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

	static_assert(sizeof(baseModel) == sizeof(databaseData->model), "Models array size mismatch");
	memcpy(this->baseModel, databaseData->model, sizeof(this->baseModel));

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

	if(true) {
		stats.nAttackPointRight = 2.8f * stats.strength + level;
	} else {
		stats.nAttackPointRight = 1.2f * stats.agility + 2.2f * stats.dexterity + level;
	}
	stats.nAttackPointLeft = 0;
	stats.nAccuracyRight = 0.5f * stats.dexterity + level;
	stats.nAccuracyLeft = stats.nAccuracyRight;
	stats.nMagicPoint = 2 * stats.intelligence + level;
	stats.nDefence = 1.6f * stats.vitality + level;
	stats.nAvoid = 0.5f * stats.agility + level;
	stats.nAttackSpeed = 0.1f * stats.agility + 100;
	stats.nMagicAccuracy = 0.4f * stats.wisdom + level;
	stats.nMagicDefence = 2 * stats.wisdom + level;
	stats.nMagicAvoid = 0.5f * stats.wisdom + level;
	stats.nMoveSpeed = 120;
	stats.nHPRegenPercentage = 5;
	stats.nMPRegenPercentage = 5;
	stats.nBlockChance = 0;
	stats.nBlockDefence = 0;
	stats.nCritical = 3 + stats.luck / 5;
	stats.nCastingSpeed = 100;
	stats.nHPRegenPoint = 48 + 2 * level;
	stats.nMPRegenPoint = 48 + 2 * level + 4.1f * stats.wisdom;
	stats.nCriticalPower = 80;
	stats.nCoolTimeSpeed = 100;
	stats.nMaxWeight = 10 * (stats.strength + level);
	stats.nItemChance = stats.luck / 5;
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

	maxHp = 33 * stats.vitality + 20 * level;
	maxMp = 30 * stats.intelligence + 20 * level;
	maxStamina = 500000;
	maxChaos = 500;

	statBase = stats;
	memset(&statBuffs, 0, sizeof(statBuffs));

	sendPacketStats();
}

void Character::sendPacketStats() {
	statBase.sendPacket(session, handle, StatBase::Base);
	statBuffs.sendPacket(session, handle, StatBase::Buff);

	session->sendProperty(handle, "max_chaos", maxChaos);
	session->sendProperty(handle, "max_stamina", maxStamina);
}

void Character::synchronizeWithClient() {
	sendPacketStats();

	TS_SC_INVENTORY inventory;
	for(size_t i = 0; i < items.size(); i++) {
		TS_ITEM_INFO itemInfo = {0};
		items[i]->fillInventoryItem(itemInfo);
		inventory.items.push_back(itemInfo);
		if(inventory.items.size() >= 45) {
			session->sendPacket(inventory);
			inventory.items.clear();
		}
	}
	if(inventory.items.size() > 0 || items.size() == 0)
		session->sendPacket(inventory);

	TS_SC_EQUIP_SUMMON equipSummon = {0};
	session->sendPacket(equipSummon);

	TS_SC_WEAR_INFO wearInfo = {0};
	wearInfo.handle = handle;
	for(size_t i = 0; i < items.size(); i++) {
		const Item* item = items[i].get();
		if(item->wear_info < 0)
			continue;

		if((size_t)item->wear_info < sizeof(wearInfo.wear_info) / sizeof(wearInfo.wear_info[0])) {
			wearInfo.wear_info[item->wear_info] = item->code;
			wearInfo.wear_item_level_info[item->wear_info] = item->level;
			wearInfo.wear_item_enhance_info[item->wear_info] = item->enhance;
			wearInfo.wear_item_elemental_type[item->wear_info] = item->elemental_effect_type;
			wearInfo.wear_appearance_code[item->wear_info] = item->appearance_code;
		}
	}
	if(!wearInfo.wear_info[Item::WEAR_ARMOR])
		wearInfo.wear_info[Item::WEAR_ARMOR] = baseModel[2];
	if(!wearInfo.wear_info[Item::WEAR_GLOVE])
		wearInfo.wear_info[Item::WEAR_GLOVE] = baseModel[3];
	if(!wearInfo.wear_info[Item::WEAR_BOOTS])
		wearInfo.wear_info[Item::WEAR_BOOTS] = baseModel[4];
	session->sendPacket(wearInfo);

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
