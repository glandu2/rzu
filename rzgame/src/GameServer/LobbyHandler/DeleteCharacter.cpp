#include "DeleteCharacter.h"
#include "../../GlobalConfig.h"
#include "Database/DbQueryJob.h"

template<> void DbQueryJob<GameServer::DeleteCharacterBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "{CALL smp_delete_character(?, ?)}",
				  DbQueryBinding::EM_NoRow);

	addParam("name", &InputType::character_name);
	addOutputParam("sid", &InputType::out_character_sid);
}
DECLARE_DB_BINDING(GameServer::DeleteCharacterBinding, "delete_character");
