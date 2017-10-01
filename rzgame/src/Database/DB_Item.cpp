#include "DB_Item.h"
#include "Config/GlobalConfig.h"

template<> void DbQueryJob<GameServer::DB_ItemBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->game.telecaster.connectionString,
	              "select * from item where account_id = 0 AND owner_id = ? AND auction_id = 0 AND keeping_id = 0",
	              DbQueryBinding::EM_MultiRows);

	addParam("owner_id", &InputType::owner_id);

	addColumn("sid", &OutputType::sid);
	addColumn("owner_id", &OutputType::owner_id);
	addColumn("summon_id", &OutputType::summon_id);
	addColumn("idx", &OutputType::idx);
	addColumn("code", &OutputType::code);
	addColumn("cnt", &OutputType::count);
	addColumn("level", &OutputType::level);
	addColumn("enhance", &OutputType::enhance);
	addColumn("ethereal_durability", &OutputType::ethereal_durability);
	addColumn("endurance", &OutputType::endurance);
	addColumn("flag", &OutputType::flag);
	addColumn("gcode", &OutputType::gcode);
	addColumn("wear_info", &OutputType::wear_info);
	addColumn("socket_0", &OutputType::socket, 0);
	addColumn("socket_1", &OutputType::socket, 1);
	addColumn("socket_2", &OutputType::socket, 2);
	addColumn("socket_3", &OutputType::socket, 3);
	addColumn("awaken_sid", &OutputType::awaken_sid);
	addColumn("remain_time", &OutputType::remain_time);
	addColumn("elemental_effect_type", &OutputType::elemental_effect_type);
	addColumn("elemental_effect_expire_time", &OutputType::elemental_effect_expire_time);
	addColumn("elemental_effect_attack_point", &OutputType::elemental_effect_attack_point);
	addColumn("elemental_effect_magic_point", &OutputType::elemental_effect_magic_point);
	addColumn("appearance_code", &OutputType::appearance_code);
	addColumn("create_time", &OutputType::create_time);
	addColumn("update_time", &OutputType::update_time);
}
DECLARE_DB_BINDING(GameServer::DB_ItemBinding, "item_data");
