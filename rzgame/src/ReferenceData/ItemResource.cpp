#include "ItemResource.h"
#include "Config/GlobalConfig.h"
#include <iterator>

template<> void DbQueryJob<GameServer::ItemResourceBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.arcadia.connectionString,
				  "select * from ItemResource",
				  DbQueryBinding::EM_MultiRows);

	addColumn("id", &OutputType::id);
	addColumn("name_id", &OutputType::name_id);
	addColumn("type", &OutputType::type);
	addColumn("group", &OutputType::group);
	addColumn("class", &OutputType::classType);
	addColumn("wear_type", &OutputType::wear_type);
	addColumn("set_id", &OutputType::set_id);
	addColumn("set_part_flag", &OutputType::set_part_flag);
	addColumn("grade", &OutputType::grade);
	addColumn("rank", &OutputType::rank);
	addColumn("level", &OutputType::level);
	addColumn("enhance", &OutputType::enhance);
	addColumn("socket", &OutputType::socket);
	addColumn("status_flag", &OutputType::status_flag);
	addColumn("limit_deva", &OutputType::limit_deva);
	addColumn("limit_asura", &OutputType::limit_asura);
	addColumn("limit_gaia", &OutputType::limit_gaia);
	addColumn("job_depth", &OutputType::job_depth);
	addColumn("limit_fighter", &OutputType::limit_fighter);
	addColumn("limit_hunter", &OutputType::limit_hunter);
	addColumn("limit_magician", &OutputType::limit_magician);
	addColumn("limit_summoner", &OutputType::limit_summoner);
	addColumn("use_min_level", &OutputType::use_min_level);
	addColumn("use_max_level", &OutputType::use_max_level);
	addColumn("target_min_level", &OutputType::target_min_level);
	addColumn("target_max_level", &OutputType::target_max_level);
	addColumn("range", &OutputType::range);
	addColumn("weight", &OutputType::weight);
	addColumn("price", &OutputType::price);
	addColumn("huntaholic_point", &OutputType::huntaholic_point);
	addColumn("arena_point", &OutputType::arena_point);
	addColumn("ethereal_durability", &OutputType::ethereal_durability);
	addColumn("endurance", &OutputType::endurance);
	addColumn("material", &OutputType::material);
	addColumn("summon_id", &OutputType::summon_id);
	addColumn("item_use_flag", &OutputType::item_use_flag);
	addColumn("available_period", &OutputType::available_period);
	addColumn("decrease_type", &OutputType::decrease_type);
	addColumn("throw_range", &OutputType::throw_range);
	addColumn("base_type_0", &OutputType::base_type, 0);
	addColumn("base_var1_0", &OutputType::base_var1, 0);
	addColumn("base_var2_0", &OutputType::base_var2, 0);
	addColumn("base_type_1", &OutputType::base_type, 1);
	addColumn("base_var1_1", &OutputType::base_var1, 1);
	addColumn("base_var2_1", &OutputType::base_var2, 1);
	addColumn("base_type_2", &OutputType::base_type, 2);
	addColumn("base_var1_2", &OutputType::base_var1, 2);
	addColumn("base_var2_2", &OutputType::base_var2, 2);
	addColumn("base_type_3", &OutputType::base_type, 3);
	addColumn("base_var1_3", &OutputType::base_var1, 3);
	addColumn("base_var2_3", &OutputType::base_var2, 3);
	addColumn("opt_type_0", &OutputType::opt_type, 0);
	addColumn("opt_var1_0", &OutputType::opt_var1, 0);
	addColumn("opt_var2_0", &OutputType::opt_var2, 0);
	addColumn("opt_type_1", &OutputType::opt_type, 1);
	addColumn("opt_var1_1", &OutputType::opt_var1, 1);
	addColumn("opt_var2_1", &OutputType::opt_var2, 1);
	addColumn("opt_type_2", &OutputType::opt_type, 2);
	addColumn("opt_var1_2", &OutputType::opt_var1, 2);
	addColumn("opt_var2_2", &OutputType::opt_var2, 2);
	addColumn("opt_type_3", &OutputType::opt_type, 3);
	addColumn("opt_var1_3", &OutputType::opt_var1, 3);
	addColumn("opt_var2_3", &OutputType::opt_var2, 3);
	addColumn("effect_id", &OutputType::effect_id);
	addColumn("enhance_id", &OutputType::enhance_id);
	addColumn("skill_id", &OutputType::skill_id);
	addColumn("state_id", &OutputType::state_id);
	addColumn("state_level", &OutputType::state_level);
	addColumn("state_time", &OutputType::state_time);
	addColumn("cool_time", &OutputType::cool_time);
	addColumn("cool_time_group", &OutputType::cool_time_group);
	addColumn("script_text", &OutputType::script_text);
}
DECLARE_DB_BINDING(GameServer::ItemResourceBinding, "itemresource");
