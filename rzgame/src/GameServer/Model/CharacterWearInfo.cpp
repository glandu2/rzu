#include "CharacterWearInfo.h"
#include "../../GlobalConfig.h"

DECLARE_DB_BINDING(GameServer::CharacterWearInfoBinding, "character_wear_info");
template<> void DbQueryJob<GameServer::CharacterWearInfoBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "SELECT owner_id, wear_info, code, enhance, level, elemental_effect_type, appearance_code "
				  "FROM Item i INNER JOIN Character c ON c.sid = i.owner_id "
				  "WHERE c.account_id = ? AND i.account_id = 0 AND i.summon_id = 0 AND i.auction_id = 0 AND i.keeping_id = 0 AND i.wear_info >= 0",
				  DbQueryBinding::EM_MultiRows);

	addParam("account_id", &InputType::account_id);

	addColumn("owner_id", &OutputType::character_sid);
	addColumn("wear_info", &OutputType::wear_info);
	addColumn("code", &OutputType::code);
	addColumn("enhance", &OutputType::enhance);
	addColumn("level", &OutputType::level);
	addColumn("elemental_effect_type", &OutputType::elemental_effect_type);
	addColumn("appearance_code", &OutputType::appearance_code);
}
