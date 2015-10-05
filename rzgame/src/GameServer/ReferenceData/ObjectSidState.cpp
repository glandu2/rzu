#include "ObjectSidState.h"
#include "../../GlobalConfig.h"
#include <iterator>

template<class T>
static void initMaxSidBinding(DbConnectionPool* dbConnectionPool, const char* query) {
	DbQueryJob<T>::createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  query,
				  DbQueryBinding::EM_OneRow);

	DbQueryJob<T>::addColumn("max_sid", &T::Output::max_sid);
}

template<> void DbQueryJob<GameServer::AwakenOptionSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::AwakenOptionSidBinding>(dbConnectionPool, "select max(sid) as max_sid from AwakenOption");
}
DECLARE_DB_BINDING(GameServer::AwakenOptionSidBinding, "max_sid_awaken_option");

template<> void DbQueryJob<GameServer::FarmSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::FarmSidBinding>(dbConnectionPool, "select max(sid) as max_sid from Farm");
}
DECLARE_DB_BINDING(GameServer::FarmSidBinding, "max_sid_farm");


template<> void DbQueryJob<GameServer::ItemSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::ItemSidBinding>(dbConnectionPool, "select max(sid) as max_sid from Item");
}
DECLARE_DB_BINDING(GameServer::ItemSidBinding, "max_sid_item");


template<> void DbQueryJob<GameServer::PetSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::PetSidBinding>(dbConnectionPool, "select max(sid) as max_sid from Pet");
}
DECLARE_DB_BINDING(GameServer::PetSidBinding, "max_sid_pet");


template<> void DbQueryJob<GameServer::SkillSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::SkillSidBinding>(dbConnectionPool, "select max(sid) as max_sid from Skill");
}
DECLARE_DB_BINDING(GameServer::SkillSidBinding, "max_sid_skill");


template<> void DbQueryJob<GameServer::SummonSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::SummonSidBinding>(dbConnectionPool, "select max(sid) as max_sid from Summon");
}
DECLARE_DB_BINDING(GameServer::SummonSidBinding, "max_sid_summon");


template<> void DbQueryJob<GameServer::TitleSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::TitleSidBinding>(dbConnectionPool, "select max(sid) as max_sid from Title");
}
DECLARE_DB_BINDING(GameServer::TitleSidBinding, "max_sid_title");


template<> void DbQueryJob<GameServer::TitleConditionSidBinding>::init(DbConnectionPool* dbConnectionPool) {
	initMaxSidBinding<GameServer::TitleConditionSidBinding>(dbConnectionPool, "select max(sid) as max_sid from TitleCondition");
}
DECLARE_DB_BINDING(GameServer::TitleConditionSidBinding, "max_sid_title_condition");
