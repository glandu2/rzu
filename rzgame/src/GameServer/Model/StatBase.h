#ifndef STATBASE_H
#define STATBASE_H

#include <stdint.h>
#include "../GameTypes.h"

namespace GameServer {

struct StatResource;
class ClientSession;

class StatBase
{
public:
	enum Type {
		Base = 0,
		Buff = 1
	};

public:
	StatBase(const StatResource* statResource = nullptr);
	void sendPacket(ClientSession *session, game_handle_t handle, Type type);

	int32_t stat_id;
	float strength;
	float vitality;
	float dexterity;
	float agility;
	float intelligence;
	float wisdom;
	float luck;

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
};

}

#endif // STATBASE_H
