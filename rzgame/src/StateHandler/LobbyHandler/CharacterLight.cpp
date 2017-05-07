#include "CharacterLight.h"
#include "Config/GlobalConfig.h"

template<> void DbQueryJob<GameServer::CharacterLightBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "{CALL smp_read_character_list(?)}",
				  DbQueryBinding::EM_MultiRows);

	addParam("account_id", &InputType::account_id);

	addColumn("sid", &OutputType::sid);
	addColumn("name", &OutputType::name);
	addColumn("race", &OutputType::race);
	addColumn("sex", &OutputType::sex);
	addColumn("lv", &OutputType::lv);
	addColumn("jlv", &OutputType::jlv);
	addColumn("exp", &OutputType::exp);
	addColumn("hp", &OutputType::hp);
	addColumn("mp", &OutputType::mp);
	addColumn("job", &OutputType::job);
	addColumn("permission", &OutputType::permission);
	addColumn("skin_color", &OutputType::skin_color);
	addColumn("model_00", &OutputType::model, 0);
	addColumn("model_01", &OutputType::model, 1);
	addColumn("model_02", &OutputType::model, 2);
	addColumn("model_03", &OutputType::model, 3);
	addColumn("model_04", &OutputType::model, 4);
	addColumn("hair_color_index", &OutputType::hair_color_index);
	addColumn("hair_color_rgb", &OutputType::hair_color_rgb);
	addColumn("hide_equip_flag", &OutputType::hide_equip_flag);
	addColumn("texture_id", &OutputType::texture_id);
	addColumn("permission", &OutputType::permission);
	addColumn("create_time", &OutputType::create_time);
	addColumn("login_time", &OutputType::login_time);
	addColumn("login_count", &OutputType::login_count);
	addColumn("logout_time", &OutputType::logout_time);
	addColumn("play_time", &OutputType::play_time);
}
DECLARE_DB_BINDING(GameServer::CharacterLightBinding, "character_list");
