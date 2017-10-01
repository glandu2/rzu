#include "CheckCharacterName.h"
#include "Config/GlobalConfig.h"
#include "Database/DbQueryJob.h"

template<> void DbQueryJob<GameServer::CheckCharacterNameBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->game.telecaster.connectionString,
	              "select 1 from Character where name = ?",
	              DbQueryBinding::EM_OneRow);

	addParam("name", &InputType::character_name);
}
DECLARE_DB_BINDING(GameServer::CheckCharacterNameBinding, "check_character_name");
