#ifndef JOBLEVELBONUS_H
#define JOBLEVELBONUS_H

#include "RefDataLoader.h"
#include "Database/DbQueryJobRef.h"
#include <unordered_map>
#include <stdint.h>

namespace GameServer {

class StatBase;


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
};

}

#endif // JOBLEVELBONUS_H
