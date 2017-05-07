#include "DB_Character.h"
#include "Config/GlobalConfig.h"

template<> void DbQueryJob<GameServer::DB_CharacterBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.telecaster.connectionString,
				  "select * from character where sid = ?",
				  DbQueryBinding::EM_OneRow);

	addParam("sid", &InputType::sid);

	addColumn("name", &OutputType::name);
	addColumn("account", &OutputType::account);
	addColumn("party_id", &OutputType::party_id);
	addColumn("permission", &OutputType::permission);
	addColumn("x", &OutputType::x);
	addColumn("y", &OutputType::y);
	addColumn("z", &OutputType::z);
	addColumn("layer", &OutputType::layer);
	addColumn("race", &OutputType::race);
	addColumn("sex", &OutputType::sex);
	addColumn("lv", &OutputType::lv);
	addColumn("exp", &OutputType::exp);
	addColumn("hp", &OutputType::hp);
	addColumn("mp", &OutputType::mp);
	addColumn("stamina", &OutputType::stamina);
	addColumn("job", &OutputType::job);
	addColumn("job_depth", &OutputType::job_depth);
	addColumn("jlv", &OutputType::jlv);
	addColumn("jp", &OutputType::jp);
	addColumn("total_jp", &OutputType::total_jp);
	addColumn("talent_point", &OutputType::talent_point);
	addColumn("job_0", &OutputType::jobs, 0);
	addColumn("job_1", &OutputType::jobs, 1);
	addColumn("job_2", &OutputType::jobs, 2);
	addColumn("jlv_0", &OutputType::jlvs, 0);
	addColumn("jlv_1", &OutputType::jlvs, 1);
	addColumn("jlv_2", &OutputType::jlvs, 2);
	addColumn("immoral_point", &OutputType::immoral_point);
	addColumn("pkc", &OutputType::pkc);
	addColumn("dkc", &OutputType::dkc);
	addColumn("huntaholic_point", &OutputType::huntaholic_point);
	addColumn("huntaholic_enter_count", &OutputType::huntaholic_enter_count);
	addColumn("ethereal_stone_durability", &OutputType::ethereal_stone_durability);
	addColumn("gold", &OutputType::gold);
	addColumn("chaos", &OutputType::chaos);
	addColumn("skin_color", &OutputType::skin_color);
	addColumn("model_00", &OutputType::model, 0);
	addColumn("model_01", &OutputType::model, 1);
	addColumn("model_02", &OutputType::model, 2);
	addColumn("model_03", &OutputType::model, 3);
	addColumn("model_04", &OutputType::model, 4);
	addColumn("hair_color_index", &OutputType::hair_color_index);
	addColumn("hair_color_rgb", &OutputType::hair_color_rgb);
	addColumn("hide_equip_flag", &OutputType::hide_equip_flag);
	addColumn("texture_id", &OutputType::face_texture_id);
	addColumn("belt_00", &OutputType::belt, 0);
	addColumn("belt_01", &OutputType::belt, 1);
	addColumn("belt_02", &OutputType::belt, 2);
	addColumn("belt_03", &OutputType::belt, 3);
	addColumn("belt_04", &OutputType::belt, 4);
	addColumn("belt_05", &OutputType::belt, 5);
	addColumn("summon_0", &OutputType::summon_id, 0);
	addColumn("summon_1", &OutputType::summon_id, 1);
	addColumn("summon_2", &OutputType::summon_id, 2);
	addColumn("summon_3", &OutputType::summon_id, 3);
	addColumn("summon_4", &OutputType::summon_id, 4);
	addColumn("summon_5", &OutputType::summon_id, 5);
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
DECLARE_DB_BINDING(GameServer::DB_CharacterBinding, "character_data");
