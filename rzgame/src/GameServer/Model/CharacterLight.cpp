#include "CharacterLight.h"
#include "../../GlobalConfig.h"

template<>
DbQueryBinding* DbQueryJob<GameServer::CharacterLightBinding>::dbBinding = nullptr;

template<>
const char* DbQueryJob<GameServer::CharacterLightBinding>::SQL_CONFIG_NAME = "character_list";

template<>
bool DbQueryJob<GameServer::CharacterLightBinding>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	addParam(params, "account_id", &InputType::account_id);

	addColumn(cols, "sid", &OutputType::sid);
	addColumn(cols, "name", &OutputType::name);
	addColumn(cols, "race", &OutputType::race);
	addColumn(cols, "sex", &OutputType::sex);
	addColumn(cols, "lv", &OutputType::lv);
	addColumn(cols, "jlv", &OutputType::jlv);
	addColumn(cols, "exp", &OutputType::exp);
	addColumn(cols, "hp", &OutputType::hp);
	addColumn(cols, "mp", &OutputType::mp);
	addColumn(cols, "job", &OutputType::job);
	addColumn(cols, "permission", &OutputType::permission);
	addColumn(cols, "skin_color", &OutputType::skin_color);
	addColumn(cols, "model_00", &OutputType::model, 0);
	addColumn(cols, "model_01", &OutputType::model, 1);
	addColumn(cols, "model_02", &OutputType::model, 2);
	addColumn(cols, "model_03", &OutputType::model, 3);
	addColumn(cols, "model_04", &OutputType::model, 4);
	addColumn(cols, "hair_color_index", &OutputType::hair_color_index);
	addColumn(cols, "hair_color_rgb", &OutputType::hair_color_rgb);
	addColumn(cols, "hide_equip_flag", &OutputType::hide_equip_flag);
	addColumn(cols, "texture_id", &OutputType::texture_id);
	addColumn(cols, "permission", &OutputType::permission);
	addColumn(cols, "create_time", &OutputType::create_time);
	addColumn(cols, "login_time", &OutputType::login_time);
	addColumn(cols, "login_count", &OutputType::login_count);
	addColumn(cols, "logout_time", &OutputType::logout_time);
	addColumn(cols, "play_time", &OutputType::play_time);

	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "{CALL smp_read_character_list(?)}",
				  params,
				  cols,
				  DbQueryBinding::EM_OneRow);

	return true;
}
