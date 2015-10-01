#include "Character.h"
#include "../../GlobalConfig.h"

template<>
DbQueryBinding* DbQueryJob<GameServer::CharacterDetailsBinding>::dbBinding = nullptr;

template<>
const char* DbQueryJob<GameServer::CharacterDetailsBinding>::SQL_CONFIG_NAME = "character_data";

template<>
bool DbQueryJob<GameServer::CharacterDetailsBinding>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	addParam(params, "sid", &InputType::sid);

	addColumn(cols, "party_id", &OutputType::party_id);
	addColumn(cols, "slot", &OutputType::slot);
	addColumn(cols, "permission", &OutputType::permission);
	addColumn(cols, "x", &OutputType::x);
	addColumn(cols, "y", &OutputType::y);
	addColumn(cols, "z", &OutputType::z);
	addColumn(cols, "layer", &OutputType::layer);
	addColumn(cols, "max_reached_level", &OutputType::max_reached_level);
	addColumn(cols, "last_decreased_exp", &OutputType::last_decreased_exp);
	addColumn(cols, "stamina", &OutputType::stamina);
	addColumn(cols, "havoc", &OutputType::havoc);
	addColumn(cols, "job_depth", &OutputType::job_depth);
	addColumn(cols, "jlv", &OutputType::current_jlv);
	addColumn(cols, "jp", &OutputType::jp);
	addColumn(cols, "total_jp", &OutputType::total_jp);
	addColumn(cols, "talent_point", &OutputType::talent_point);
	addColumn(cols, "job_0", &OutputType::job, 0);
	addColumn(cols, "job_1", &OutputType::job, 1);
	addColumn(cols, "job_2", &OutputType::job, 2);
	addColumn(cols, "jlv_0", &OutputType::jlv, 0);
	addColumn(cols, "jlv_1", &OutputType::jlv, 1);
	addColumn(cols, "jlv_2", &OutputType::jlv, 2);
	addColumn(cols, "immoral_point", &OutputType::immoral_point);
	addColumn(cols, "cha", &OutputType::cha);
	addColumn(cols, "pkc", &OutputType::pkc);
	addColumn(cols, "dkc", &OutputType::dkc);
	addColumn(cols, "huntaholic_point", &OutputType::huntaholic_point);
	addColumn(cols, "huntaholic_enter_count", &OutputType::huntaholic_enter_count);
	addColumn(cols, "ethereal_stone_durability", &OutputType::ethereal_stone_durability);
	addColumn(cols, "gold", &OutputType::gold);
	addColumn(cols, "chaos", &OutputType::chaos);
	addColumn(cols, "belt_00", &OutputType::belt, 0);
	addColumn(cols, "belt_01", &OutputType::belt, 1);
	addColumn(cols, "belt_02", &OutputType::belt, 2);
	addColumn(cols, "belt_03", &OutputType::belt, 3);
	addColumn(cols, "belt_04", &OutputType::belt, 4);
	addColumn(cols, "belt_05", &OutputType::belt, 5);
	addColumn(cols, "summon_0", &OutputType::summon, 0);
	addColumn(cols, "summon_1", &OutputType::summon, 1);
	addColumn(cols, "summon_2", &OutputType::summon, 2);
	addColumn(cols, "summon_3", &OutputType::summon, 3);
	addColumn(cols, "summon_4", &OutputType::summon, 4);
	addColumn(cols, "summon_5", &OutputType::summon, 5);
	addColumn(cols, "main_summon", &OutputType::main_summon);
	addColumn(cols, "sub_summon", &OutputType::sub_summon);
	addColumn(cols, "remain_summon_time", &OutputType::remain_summon_time);
	addColumn(cols, "pet", &OutputType::pet);
	addColumn(cols, "main_title", &OutputType::main_title);
	addColumn(cols, "sub_title_0", &OutputType::sub_title, 0);
	addColumn(cols, "sub_title_1", &OutputType::sub_title, 1);
	addColumn(cols, "sub_title_2", &OutputType::sub_title, 2);
	addColumn(cols, "sub_title_3", &OutputType::sub_title, 3);
	addColumn(cols, "sub_title_4", &OutputType::sub_title, 4);
	addColumn(cols, "remain_title_time", &OutputType::remain_title_time);
	addColumn(cols, "arena_point", &OutputType::arena_point);
	addColumn(cols, "arena_block_time", &OutputType::arena_block_time);
	addColumn(cols, "arena_penalty_count", &OutputType::arena_penalty_count);
	addColumn(cols, "arena_penalty_dec_time", &OutputType::arena_penalty_dec_time);
	addColumn(cols, "arena_mvp_count", &OutputType::arena_mvp_count);
	addColumn(cols, "arena_record_0_0", &OutputType::arena_record, 0, 0);
	addColumn(cols, "arena_record_0_1", &OutputType::arena_record, 0, 1);
	addColumn(cols, "arena_record_1_0", &OutputType::arena_record, 1, 0);
	addColumn(cols, "arena_record_1_1", &OutputType::arena_record, 1, 1);
	addColumn(cols, "arena_record_2_0", &OutputType::arena_record, 2, 0);
	addColumn(cols, "arena_record_2_1", &OutputType::arena_record, 2, 1);
	addColumn(cols, "alias", &OutputType::alias);
	addColumn(cols, "chat_block_time", &OutputType::chat_block_time);
	addColumn(cols, "adv_chat_count", &OutputType::adv_chat_count);
	addColumn(cols, "name_changed", &OutputType::name_changed);
	addColumn(cols, "auto_used", &OutputType::auto_used);
	addColumn(cols, "pkmode", &OutputType::pkmode);
	addColumn(cols, "otp_value", &OutputType::otp_value);
	addColumn(cols, "otp_date", &OutputType::otp_date);
	addColumn(cols, "flag_list", &OutputType::flag_list);
	addColumn(cols, "client_info", &OutputType::client_info);

	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "select * from character where sid = ?",
				  params,
				  cols,
				  DbQueryBinding::EM_OneRow);

	return true;
}
