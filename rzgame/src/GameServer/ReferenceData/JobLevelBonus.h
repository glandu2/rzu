#ifndef JOBLEVELBONUS_H
#define JOBLEVELBONUS_H

#include "RefDataLoader.h"
#include "Database/DbQueryJobRef.h"
#include <unordered_map>
#include <stdint.h>
#include "../Model/StatBase.h"

namespace GameServer {

struct JobLevelBonus {
	int32_t id;
	int32_t strength[3];
	int32_t vitality[3];
	int32_t dexterity[3];
	int32_t agility[3];
	int32_t intelligence[3];
	int32_t wisdom[3];
	int32_t luck[3];
	int32_t default_strength;
	int32_t default_vitality;
	int32_t default_dexterity;
	int32_t default_agility;
	int32_t default_intelligence;
	int32_t default_wisdom;
	int32_t default_luck;
	//int32_t text_id;
	//int32_t icon_id;
	//std::string icon_file_name;
};

class JobLevelBonusBinding : public RefDataLoaderHelper<JobLevelBonus, JobLevelBonusBinding> {
	DECLARE_CLASS(GameServer::JobLevelBonusBinding)
public:
	void applyJobLevelBonus(const int32_t *jobs, const int32_t *jobLevels, size_t size, StatBase *statBase);

protected:
	void fillLevelParts(int32_t jobLevel, int32_t* jobLevelParts, size_t size, int32_t partLevelLength);
	int32_t computeSumProduct(const int32_t* a1, const int32_t* a2, size_t size);
};

}

#endif // JOBLEVELBONUS_H
