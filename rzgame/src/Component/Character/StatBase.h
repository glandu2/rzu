#ifndef STATBASE_H
#define STATBASE_H

#include "Core/Object.h"
#include "GameTypes.h"
#include <stddef.h>
#include <stdint.h>

namespace GameServer {

struct StatResource;
class ClientSession;

class StatBase {
public:
	enum Type { Base = 0, Buff = 1 };

public:
	int32_t stat_id;
	struct {
		float strength;
		float vitality;
		float dexterity;
		float agility;
		float intelligence;
		float wisdom;
		float luck;
	} base;

	struct {
		float nCritical;
		float nCriticalPower;
		float nAttackPointRight;
		float nAttackPointLeft;
		float nDefence;
		float nBlockDefence;
		float nMagicPoint;
		float nMagicDefence;
		float nAccuracyRight;
		float nAccuracyLeft;
		float nMagicAccuracy;
		float nAvoid;
		float nMagicAvoid;
		float nBlockChance;
		float nMoveSpeed;
		float nAttackSpeed;
		float nAttackRange;
		float nMaxWeight;
		float nCastingSpeed;
		float nCoolTimeSpeed;
		float nItemChance;
		float nHPRegenPercentage;
		float nHPRegenPoint;
		float nMPRegenPercentage;
		float nMPRegenPoint;
		float nPerfectBlock;
		float nMagicalDefIgnore;
		float nMagicalDefIgnoreRatio;
		float nPhysicalDefIgnore;
		float nPhysicalDefIgnoreRatio;
		float nMagicalPenetration;
		float nMagicalPenetrationRatio;
		float nPhysicalPenetration;
		float nPhysicalPenetrationRatio;
	} extended;

	struct {
		float maxHp;
		float maxMp;
		float maxStamina;
		float maxChaos;
	} properties;

public:
	StatBase(const StatResource* statResource = nullptr);
	void sendPacket(ClientSession* session, game_handle_t handle, Type type);

	/**
	 * @brief Apply job related stats bonuses
	 *
	 * This function retrieve the JobLevelBonus info to compute the stats for each job's levels.
	 *
	 * @param jobs jobs ids
	 * @param jobLevels jobs level
	 * @param size number of jobs
	 */
	void applyJobLevelBonus(const int32_t* jobs, const int32_t* jobLevels, size_t size);

protected:
	static void fillLevelParts(int32_t jobLevel, int32_t* jobLevelParts, size_t size, int32_t partLevelLength);
	static int32_t computeSumProduct(const int32_t* a1, const int32_t* a2, size_t size);
};

}  // namespace GameServer

#endif  // STATBASE_H
