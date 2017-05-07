#include "StatBase.h"
#include "ReferenceData/StatResource.h"
#include "ReferenceData/JobLevelBonus.h"
#include "GameClient/TS_SC_STAT_INFO.h"
#include "ClientSession.h"
#include <Core/Utils.h>

namespace GameServer {

StatBase::StatBase(const StatResource *statResource) {
	if(statResource) {
		stat_id = statResource->id;
		base.strength = (float)statResource->strength;
		base.vitality = (float)statResource->vitality;
		base.dexterity = (float)statResource->dexterity;
		base.agility = (float)statResource->agility;
		base.intelligence = (float)statResource->intelligence;
		base.wisdom = (float)statResource->wisdom;
		base.luck = (float)statResource->luck;

	} else {
		stat_id = 0;
		base = {};
	}

	extended = {};
	properties = {};
}

void StatBase::sendPacket(ClientSession* session, game_handle_t handle, Type type) {
	TS_SC_STAT_INFO statPacket;
	statPacket.handle = handle;
	statPacket.type = (int8_t)type;

	statPacket.stat.stat_id = stat_id;
	statPacket.stat.strength = (int16_t)base.strength;
	statPacket.stat.vital = (int16_t)base.vitality;
	statPacket.stat.dexterity = (int16_t)base.dexterity;
	statPacket.stat.agility = (int16_t)base.agility;
	statPacket.stat.intelligence = (int16_t)base.intelligence;
	statPacket.stat.mentality = (int16_t)base.wisdom;
	statPacket.stat.luck = (int16_t)base.luck;

	statPacket.attribute.nCritical = (int16_t)extended.nCritical;
	statPacket.attribute.nCriticalPower = (int16_t)extended.nCriticalPower;
	statPacket.attribute.nAttackPointRight = (int16_t)extended.nAttackPointRight;
	statPacket.attribute.nAttackPointLeft = (int16_t)extended.nAttackPointLeft;
	statPacket.attribute.nDefence = (int16_t)extended.nDefence;
	statPacket.attribute.nBlockDefence = (int16_t)extended.nBlockDefence;
	statPacket.attribute.nMagicPoint = (int16_t)extended.nMagicPoint;
	statPacket.attribute.nMagicDefence = (int16_t)extended.nMagicDefence;
	statPacket.attribute.nAccuracyRight = (int16_t)extended.nAccuracyRight;
	statPacket.attribute.nAccuracyLeft = (int16_t)extended.nAccuracyLeft;
	statPacket.attribute.nMagicAccuracy = (int16_t)extended.nMagicAccuracy;
	statPacket.attribute.nAvoid = (int16_t)extended.nAvoid;
	statPacket.attribute.nMagicAvoid = (int16_t)extended.nMagicAvoid;
	statPacket.attribute.nBlockChance = (int16_t)extended.nBlockChance;
	statPacket.attribute.nMoveSpeed = (int16_t)extended.nMoveSpeed;
	statPacket.attribute.nAttackSpeed = (int16_t)extended.nAttackSpeed;
	statPacket.attribute.nAttackRange = (int16_t)extended.nAttackRange;
	statPacket.attribute.nMaxWeight = (int16_t)extended.nMaxWeight;
	statPacket.attribute.nCastingSpeed = (int16_t)extended.nCastingSpeed;
	statPacket.attribute.nCoolTimeSpeed = (int16_t)extended.nCoolTimeSpeed;
	statPacket.attribute.nItemChance = (int16_t)extended.nItemChance;
	statPacket.attribute.nHPRegenPercentage = (int16_t)extended.nHPRegenPercentage;
	statPacket.attribute.nHPRegenPoint = (int16_t)extended.nHPRegenPoint;
	statPacket.attribute.nMPRegenPercentage = (int16_t)extended.nMPRegenPercentage;
	statPacket.attribute.nMPRegenPoint = (int16_t)extended.nMPRegenPoint;
	statPacket.attribute.nPerfectBlock = (int16_t)extended.nPerfectBlock;
	statPacket.attribute.nMagicalDefIgnore = (int16_t)extended.nMagicalDefIgnore;
	statPacket.attribute.nMagicalDefIgnoreRatio = (int16_t)extended.nMagicalDefIgnoreRatio;
	statPacket.attribute.nPhysicalDefIgnore = (int16_t)extended.nPhysicalDefIgnore;
	statPacket.attribute.nPhysicalDefIgnoreRatio = (int16_t)extended.nPhysicalDefIgnoreRatio;
	statPacket.attribute.nMagicalPenetration = (int16_t)extended.nMagicalPenetration;
	statPacket.attribute.nMagicalPenetrationRatio = (int16_t)extended.nMagicalPenetrationRatio;
	statPacket.attribute.nPhysicalPenetration = (int16_t)extended.nPhysicalPenetration;
	statPacket.attribute.nPhysicalPenetrationRatio = (int16_t)extended.nPhysicalPenetrationRatio;

	session->sendPacket(statPacket);
}

/**
 * @brief Compute number of levels achieved for each joblevel parts
 *
 * The number of levels for the last chunk may be above the chunk's number
 * of levels if the jobLevel is greater than the maximum expected level.
 *
 * Example: for chunk of 20 levels with 3 chunks, a level of 35 will output these chunks:
 * - First chunk: 20
 * - Second chunk: 15
 * - Third chunk: 0
 *
 * Example: for chunk of 20 levels with 3 chunks, a level of 62 will output these chunks:
 * - First chunk: 20
 * - Second chunk: 20
 * - Third chunk: 22 (above 20 because it's the last chunk)
 *
 * @param jobLevel The jobLevel to split in chunks
 * @param [out] jobLevelParts output chunks of levels
 * @param size number of chunks
 * @param partLevelLength number of level in each chunks
 */
void StatBase::fillLevelParts(int32_t jobLevel, int32_t *jobLevelParts, size_t size, int32_t partLevelLength) {
	for(size_t i = 0; i < size; i++) {
		int part;

		if(jobLevel < partLevelLength || i == size - 1)
			part = jobLevel;
		else
			part = partLevelLength;

		jobLevel -= part;
		jobLevelParts[i] = part;
	}
}

int32_t StatBase::computeSumProduct(const int32_t *a1, const int32_t *a2, size_t size) {
	int32_t sum = 0;
	for(size_t i = 0; i < size; i++) {
		sum += a1[i] * a2[i];
	}

	return sum;
}

void StatBase::applyJobLevelBonus(const int32_t* jobs, const int32_t* jobLevels, size_t size) {
	static const int32_t LEVELS_PER_JOBLEVEL_RANGE = 20;

	for(size_t i = 0; i < size; i++) {
		const JobLevelBonus* jobBonus = JobLevelBonusBinding::getData(jobs[i]);
		if(jobBonus == nullptr) {
			Object::logStatic(Object::LL_Error, "StatBase", "Couldn't compute stat bonus for jobid %d, job not found in JobLevelBonus\n", jobs[i]);
			continue;
		}

		// Each JobLevelBonus describe each stats bonus per level.
		// Each stats have 3 parts, the job level is split in 3 part:
		// - Levels 0-19
		// - Levels 20-39
		// - Levels 40-59
		// The stat bonus is different for each level ranges.

		// The effective bonus is number of levels achieved in a range multiplied by the job stat bonus
		// Then this is done for each level ranges and summed.

		int32_t levelParts[Utils_countOf(jobBonus->strength)] = {0};
		const size_t partNum = sizeof(levelParts)/sizeof(levelParts[0]);

		static_assert(sizeof(levelParts) == sizeof(jobBonus->strength), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->vitality), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->dexterity), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->agility), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->intelligence), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->wisdom), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->luck), "Stat array mistmatch");

		// Compute acheived levels for each level ranges of 20 levels
		fillLevelParts(jobLevels[i], levelParts, partNum, LEVELS_PER_JOBLEVEL_RANGE);

		this->base.strength += computeSumProduct(jobBonus->strength, levelParts, partNum) + jobBonus->default_strength;
		this->base.vitality += computeSumProduct(jobBonus->vitality, levelParts, partNum) + jobBonus->default_vitality;
		this->base.dexterity += computeSumProduct(jobBonus->dexterity, levelParts, partNum) + jobBonus->default_dexterity;
		this->base.agility += computeSumProduct(jobBonus->agility, levelParts, partNum) + jobBonus->default_agility;
		this->base.intelligence += computeSumProduct(jobBonus->intelligence, levelParts, partNum) + jobBonus->default_intelligence;
		this->base.wisdom += computeSumProduct(jobBonus->wisdom, levelParts, partNum) + jobBonus->default_wisdom;
		this->base.luck += computeSumProduct(jobBonus->luck, levelParts, partNum) + jobBonus->default_luck;
	}
}

}
