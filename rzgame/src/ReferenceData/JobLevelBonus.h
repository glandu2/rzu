#ifndef JOBLEVELBONUS_H
#define JOBLEVELBONUS_H

#include "Database/DbQueryJobRef.h"
#include "RefDataLoader.h"
#include <stdint.h>
#include <unordered_map>

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
	// int32_t text_id;
	// int32_t icon_id;
	// std::string icon_file_name;
};

class JobLevelBonusBinding : public RefDataLoaderHelper<JobLevelBonus, JobLevelBonusBinding> {
	DECLARE_CLASS(GameServer::JobLevelBonusBinding)
};

}  // namespace GameServer

#endif  // JOBLEVELBONUS_H
