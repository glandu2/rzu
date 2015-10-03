#include "Character.h"
#include "../../GlobalConfig.h"

DECLARE_DB_BINDING(GameServer::CharacterDetailsBinding, "character_data");
template<> void DbQueryJob<GameServer::CharacterDetailsBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "select * from character where sid = ?",
				  DbQueryBinding::EM_OneRow);

	addParam("sid", &InputType::sid);

	addColumn("party_id", &OutputType::party_id);
	addColumn("slot", &OutputType::slot);
	addColumn("permission", &OutputType::permission);
	addColumn("x", &OutputType::x);
	addColumn("y", &OutputType::y);
	addColumn("z", &OutputType::z);
	addColumn("layer", &OutputType::layer);
	addColumn("max_reached_level", &OutputType::max_reached_level);
	addColumn("last_decreased_exp", &OutputType::last_decreased_exp);
	addColumn("stamina", &OutputType::stamina);
	addColumn("havoc", &OutputType::havoc);
	addColumn("job_depth", &OutputType::job_depth);
	addColumn("jlv", &OutputType::current_jlv);
	addColumn("jp", &OutputType::jp);
	addColumn("total_jp", &OutputType::total_jp);
	addColumn("talent_point", &OutputType::talent_point);
	addColumn("job_0", &OutputType::job, 0);
	addColumn("job_1", &OutputType::job, 1);
	addColumn("job_2", &OutputType::job, 2);
	addColumn("jlv_0", &OutputType::jlv, 0);
	addColumn("jlv_1", &OutputType::jlv, 1);
	addColumn("jlv_2", &OutputType::jlv, 2);
	addColumn("immoral_point", &OutputType::immoral_point);
	addColumn("cha", &OutputType::cha);
	addColumn("pkc", &OutputType::pkc);
	addColumn("dkc", &OutputType::dkc);
	addColumn("huntaholic_point", &OutputType::huntaholic_point);
	addColumn("huntaholic_enter_count", &OutputType::huntaholic_enter_count);
	addColumn("ethereal_stone_durability", &OutputType::ethereal_stone_durability);
	addColumn("gold", &OutputType::gold);
	addColumn("chaos", &OutputType::chaos);
	addColumn("belt_00", &OutputType::belt, 0);
	addColumn("belt_01", &OutputType::belt, 1);
	addColumn("belt_02", &OutputType::belt, 2);
	addColumn("belt_03", &OutputType::belt, 3);
	addColumn("belt_04", &OutputType::belt, 4);
	addColumn("belt_05", &OutputType::belt, 5);
	addColumn("summon_0", &OutputType::summon, 0);
	addColumn("summon_1", &OutputType::summon, 1);
	addColumn("summon_2", &OutputType::summon, 2);
	addColumn("summon_3", &OutputType::summon, 3);
	addColumn("summon_4", &OutputType::summon, 4);
	addColumn("summon_5", &OutputType::summon, 5);
	addColumn("main_summon", &OutputType::main_summon);
	addColumn("sub_summon", &OutputType::sub_summon);
	addColumn("remain_summon_time", &OutputType::remain_summon_time);
	addColumn("pet", &OutputType::pet);
	addColumn("main_title", &OutputType::main_title);
	addColumn("sub_title_0", &OutputType::sub_title, 0);
	addColumn("sub_title_1", &OutputType::sub_title, 1);
	addColumn("sub_title_2", &OutputType::sub_title, 2);
	addColumn("sub_title_3", &OutputType::sub_title, 3);
	addColumn("sub_title_4", &OutputType::sub_title, 4);
	addColumn("remain_title_time", &OutputType::remain_title_time);
	addColumn("arena_point", &OutputType::arena_point);
	addColumn("arena_block_time", &OutputType::arena_block_time);
	addColumn("arena_penalty_count", &OutputType::arena_penalty_count);
	addColumn("arena_penalty_dec_time", &OutputType::arena_penalty_dec_time);
	addColumn("arena_mvp_count", &OutputType::arena_mvp_count);
	addColumn("arena_record_0_0", &OutputType::arena_record, 0, 0);
	addColumn("arena_record_0_1", &OutputType::arena_record, 0, 1);
	addColumn("arena_record_1_0", &OutputType::arena_record, 1, 0);
	addColumn("arena_record_1_1", &OutputType::arena_record, 1, 1);
	addColumn("arena_record_2_0", &OutputType::arena_record, 2, 0);
	addColumn("arena_record_2_1", &OutputType::arena_record, 2, 1);
	addColumn("alias", &OutputType::alias);
	addColumn("chat_block_time", &OutputType::chat_block_time);
	addColumn("adv_chat_count", &OutputType::adv_chat_count);
	addColumn("name_changed", &OutputType::name_changed);
	addColumn("auto_used", &OutputType::auto_used);
	addColumn("pkmode", &OutputType::pkmode);
	addColumn("otp_value", &OutputType::otp_value);
	addColumn("otp_date", &OutputType::otp_date);
	addColumn("flag_list", &OutputType::flag_list);
	addColumn("client_info", &OutputType::client_info);
}
