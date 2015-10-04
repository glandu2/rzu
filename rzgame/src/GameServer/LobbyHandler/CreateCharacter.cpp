#include "CreateCharacter.h"
#include "../../GlobalConfig.h"
#include "Database/DbQueryJob.h"

DECLARE_DB_BINDING(GameServer::CreateCharacterBinding, "check_character_name");
template<> void DbQueryJob<GameServer::CreateCharacterBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "{CALL smp_insert_character(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}",
				  DbQueryBinding::EM_NoRow);

	addOutputParam("out_character_sid", &InputType::out_character_sid);

	addParam("name", &InputType::name);
	addParam("account_name", &InputType::account_name);
	addParam("account_id", &InputType::account_id);
	addParam("slot", &InputType::slot);
	addParam("x", &InputType::x);
	addParam("y", &InputType::y);
	addParam("z", &InputType::z);
	addParam("layer", &InputType::layer);
	addParam("race", &InputType::race);
	addParam("sex", &InputType::sex);
	addParam("lv", &InputType::level);
	addParam("max_reached_level", &InputType::max_reached_level);
	addParam("hp", &InputType::hp);
	addParam("mp", &InputType::mp);
	addParam("jlv", &InputType::jlv);
	addParam("jp", &InputType::jp);
	addParam("cha", &InputType::cha);
	addParam("skin_color", &InputType::skin_color);
	addParam("model_00", &InputType::model, 0);
	addParam("model_01", &InputType::model, 1);
	addParam("model_02", &InputType::model, 2);
	addParam("model_03", &InputType::model, 3);
	addParam("model_04", &InputType::model, 4);
	addParam("hair_color_index", &InputType::hair_color_index);
	addParam("texture_id", &InputType::texture_id);
	addParam("default_weapon_sid", &InputType::default_weapon_sid);
	addParam("default_weapon_code", &InputType::default_weapon_code);
	addParam("default_armor_sid", &InputType::default_armor_sid);
	addParam("default_armor_code", &InputType::default_armor_code);
	addParam("default_bag_sid", &InputType::default_bag_sid);
	addParam("default_bag_code", &InputType::default_bag_code);
}
