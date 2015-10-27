#include "JobLevelBonus.h"
#include "../../GlobalConfig.h"
#include <iterator>

template<> void DbQueryJob<GameServer::JobLevelBonusBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.arcadia.connectionString,
				  "select * from JobLevelBonus",
				  DbQueryBinding::EM_MultiRows);

	addColumn("job_id", &OutputType::id);

	addColumn("str_1", &OutputType::strength, 0);
	addColumn("str_2", &OutputType::strength, 1);
	addColumn("str_3", &OutputType::strength, 2);
	addColumn("vit_1", &OutputType::vitality, 0);
	addColumn("vit_2", &OutputType::vitality, 1);
	addColumn("vit_3", &OutputType::vitality, 2);
	addColumn("dex_1", &OutputType::dexterity, 0);
	addColumn("dex_2", &OutputType::dexterity, 1);
	addColumn("dex_3", &OutputType::dexterity, 2);
	addColumn("agi_1", &OutputType::agility, 0);
	addColumn("agi_2", &OutputType::agility, 1);
	addColumn("agi_3", &OutputType::agility, 2);
	addColumn("int_1", &OutputType::intelligence, 0);
	addColumn("int_2", &OutputType::intelligence, 1);
	addColumn("int_3", &OutputType::intelligence, 2);
	addColumn("men_1", &OutputType::wisdom, 0);
	addColumn("men_2", &OutputType::wisdom, 1);
	addColumn("men_3", &OutputType::wisdom, 2);
	addColumn("luk_1", &OutputType::luck, 0);
	addColumn("luk_2", &OutputType::luck, 1);
	addColumn("luk_3", &OutputType::luck, 2);
	addColumn("default_str", &OutputType::default_strength);
	addColumn("default_vit", &OutputType::default_vitality);
	addColumn("default_dex", &OutputType::default_dexterity);
	addColumn("default_agi", &OutputType::default_agility);
	addColumn("default_int", &OutputType::default_intelligence);
	addColumn("default_men", &OutputType::default_wisdom);
	addColumn("default_luk", &OutputType::default_luck);
}
DECLARE_DB_BINDING(GameServer::JobLevelBonusBinding, "joblevelbonus");

namespace GameServer {

void JobLevelBonusBinding::fillLevelParts(int32_t jobLevel, int32_t *jobLevelParts, size_t size, int32_t partLevelLength) {
	for(size_t i = 0; i < size; i++) {
		int part = jobLevel < partLevelLength ? jobLevel : partLevelLength;
		jobLevel -= part;
		jobLevelParts[i] = part;
	}
}

int32_t JobLevelBonusBinding::computeSumProduct(const int32_t *a1, const int32_t *a2, size_t size) {
	int32_t sum = 0;
	for(size_t i = 0; i < size; i++) {
		sum += a1[i] * a2[i];
	}

	return sum;
}

void JobLevelBonusBinding::applyJobLevelBonus(const int32_t* jobs, const int32_t* jobLevels, size_t size, StatBase* statBase) {
	for(size_t i = 0; i < size; i++) {
		const JobLevelBonus* jobBonus = getData(jobs[i]);
		if(jobBonus == nullptr) {
			log(LL_Error, "Couldn't compute stat bonus for jobid %d, job not found in JobLevelBonus\n", jobs[i]);
			continue;
		}

		int32_t levelParts[3] = {0};
		const size_t partNum = sizeof(levelParts)/sizeof(levelParts[0]);

		static_assert(sizeof(levelParts) == sizeof(jobBonus->strength), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->vitality), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->dexterity), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->agility), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->intelligence), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->wisdom), "Stat array mistmatch");
		static_assert(sizeof(levelParts) == sizeof(jobBonus->luck), "Stat array mistmatch");

		// Separate level in chunks of 20 levels
		fillLevelParts(jobLevels[i], levelParts, partNum, 20);

		statBase->strength += computeSumProduct(jobBonus->strength, levelParts, partNum) + jobBonus->default_strength;
		statBase->vitality += computeSumProduct(jobBonus->vitality, levelParts, partNum) + jobBonus->default_vitality;
		statBase->dexterity += computeSumProduct(jobBonus->dexterity, levelParts, partNum) + jobBonus->default_dexterity;
		statBase->agility += computeSumProduct(jobBonus->agility, levelParts, partNum) + jobBonus->default_agility;
		statBase->intelligence += computeSumProduct(jobBonus->intelligence, levelParts, partNum) + jobBonus->default_intelligence;
		statBase->wisdom += computeSumProduct(jobBonus->wisdom, levelParts, partNum) + jobBonus->default_wisdom;
		statBase->luck += computeSumProduct(jobBonus->luck, levelParts, partNum) + jobBonus->default_luck;
	}
}

}
