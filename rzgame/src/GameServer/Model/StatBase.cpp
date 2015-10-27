#include "StatBase.h"
#include "../ReferenceData/StatResource.h"
#include "GameClient/TS_SC_STAT_INFO.h"
#include "../ClientSession.h"

namespace GameServer {

StatBase::StatBase(const StatResource *statResource) {
	if(statResource) {
		stat_id = statResource->id;
		strength = (float)statResource->strength;
		vitality = (float)statResource->vitality;
		dexterity = (float)statResource->dexterity;
		agility = (float)statResource->agility;
		intelligence = (float)statResource->intelligence;
		wisdom = (float)statResource->wisdom;
		luck = (float)statResource->luck;
	} else {
		stat_id = 0;
	}
}

void StatBase::sendPacket(ClientSession* session, game_handle_t handle, Type type) {
	TS_SC_STAT_INFO statPacket;
	statPacket.handle = handle;
	statPacket.type = (int8_t)type;

	statPacket.stat.strength = (int16_t)strength;
	statPacket.stat.vital = (int16_t)vitality;
	statPacket.stat.dexterity = (int16_t)dexterity;
	statPacket.stat.agility = (int16_t)agility;
	statPacket.stat.intelligence = (int16_t)intelligence;
	statPacket.stat.mentality = (int16_t)wisdom;
	statPacket.stat.luck = (int16_t)luck;

	statPacket.attribute.nCritical = (int16_t)nCritical;
	statPacket.attribute.nCriticalPower = (int16_t)nCriticalPower;
	statPacket.attribute.nAttackPointRight = (int16_t)nAttackPointRight;
	statPacket.attribute.nAttackPointLeft = (int16_t)nAttackPointLeft;
	statPacket.attribute.nDefence = (int16_t)nDefence;
	statPacket.attribute.nBlockDefence = (int16_t)nBlockDefence;
	statPacket.attribute.nMagicPoint = (int16_t)nMagicPoint;
	statPacket.attribute.nMagicDefence = (int16_t)nMagicDefence;
	statPacket.attribute.nAccuracyRight = (int16_t)nAccuracyRight;
	statPacket.attribute.nAccuracyLeft = (int16_t)nAccuracyLeft;
	statPacket.attribute.nMagicAccuracy = (int16_t)nMagicAccuracy;
	statPacket.attribute.nAvoid = (int16_t)nAvoid;
	statPacket.attribute.nMagicAvoid = (int16_t)nMagicAvoid;
	statPacket.attribute.nBlockChance = (int16_t)nBlockChance;
	statPacket.attribute.nMoveSpeed = (int16_t)nMoveSpeed;
	statPacket.attribute.nAttackSpeed = (int16_t)nAttackSpeed;
	statPacket.attribute.nAttackRange = (int16_t)nAttackRange;
	statPacket.attribute.nMaxWeight = (int16_t)nMaxWeight;
	statPacket.attribute.nCastingSpeed = (int16_t)nCastingSpeed;
	statPacket.attribute.nCoolTimeSpeed = (int16_t)nCoolTimeSpeed;
	statPacket.attribute.nItemChance = (int16_t)nItemChance;
	statPacket.attribute.nHPRegenPercentage = (int16_t)nHPRegenPercentage;
	statPacket.attribute.nHPRegenPoint = (int16_t)nHPRegenPoint;
	statPacket.attribute.nMPRegenPercentage = (int16_t)nMPRegenPercentage;
	statPacket.attribute.nMPRegenPoint = (int16_t)nMPRegenPoint;
	statPacket.attribute.nPerfectBlock = (int16_t)nPerfectBlock;
	statPacket.attribute.nMagicalDefIgnore = (int16_t)nMagicalDefIgnore;
	statPacket.attribute.nMagicalDefIgnoreRatio = (int16_t)nMagicalDefIgnoreRatio;
	statPacket.attribute.nPhysicalDefIgnore = (int16_t)nPhysicalDefIgnore;
	statPacket.attribute.nPhysicalDefIgnoreRatio = (int16_t)nPhysicalDefIgnoreRatio;
	statPacket.attribute.nMagicalPenetration = (int16_t)nMagicalPenetration;
	statPacket.attribute.nMagicalPenetrationRatio = (int16_t)nMagicalPenetrationRatio;
	statPacket.attribute.nPhysicalPenetration = (int16_t)nPhysicalPenetration;
	statPacket.attribute.nPhysicalPenetrationRatio = (int16_t)nPhysicalPenetrationRatio;

	session->sendPacket(statPacket);
}

}
