#include "JobLevelBonus.h"
#include "Config/GlobalConfig.h"
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
