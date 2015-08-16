#include "CharacterList.h"
#include "../../GlobalConfig.h"
#include "../ClientSession.h"

template<>
DbQueryBinding* DbQueryJob<Database::CharacterList>::dbBinding = nullptr;

template<>
bool DbQueryJob<Database::CharacterList>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	ADD_PARAM(params, "character_list", account_id, 0, 1);

	ADD_COLUMN(cols, "character_list", sid, 0);
	ADD_COLUMN(cols, "character_list", name, 61);
	ADD_COLUMN(cols, "character_list", race, 0);
	ADD_COLUMN(cols, "character_list", sex, 0);
	ADD_COLUMN(cols, "character_list", lv, 0);
	ADD_COLUMN(cols, "character_list", jlv, 0);
	ADD_COLUMN(cols, "character_list", exp, 0);
	ADD_COLUMN(cols, "character_list", hp, 0);
	ADD_COLUMN(cols, "character_list", mp, 0);
	ADD_COLUMN(cols, "character_list", job, 0);
	ADD_COLUMN(cols, "character_list", permission, 0);
	ADD_COLUMN(cols, "character_list", skin_color, 0);
	ADD_COLUMN(cols, "character_list", model_00, 0);
	ADD_COLUMN(cols, "character_list", model_01, 0);
	ADD_COLUMN(cols, "character_list", model_02, 0);
	ADD_COLUMN(cols, "character_list", model_03, 0);
	ADD_COLUMN(cols, "character_list", model_04, 0);
	ADD_COLUMN(cols, "character_list", hair_color_index, 0);
	ADD_COLUMN(cols, "character_list", hair_color_rgb, 0);
	ADD_COLUMN(cols, "character_list", hide_equip_flag, 0);
	ADD_COLUMN(cols, "character_list", texture_id, 0);
	ADD_COLUMN(cols, "character_list", permission, 0);
	ADD_COLUMN(cols, "character_list", create_time, 0);
	ADD_COLUMN(cols, "character_list", login_time, 0);
	ADD_COLUMN(cols, "character_list", login_count, 0);
	ADD_COLUMN(cols, "character_list", logout_time, 0);
	ADD_COLUMN(cols, "character_list", play_time, 0);

	dbBinding = new DbQueryBinding(dbConnectionPool,
								   CFG_CREATE("sql.character_list.enable", true),
								   CONFIG_GET()->game.db.connectionString,
								   CFG_CREATE("sql.character_list.query", "{CALL smp_read_character_list(?)}"),
								   params,
								   cols,
								   DbQueryBinding::EM_OneRow);

	return true;
}
