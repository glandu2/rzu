#include "AuctionSQLWriter.h"

cval<std::string>& connectionString = CFG_CREATE("connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

template<> void DbQueryJob<DB_Item>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  connectionString,
				  "INSERT INTO auctions ("
				  "\"uid\", "
				  "\"diff_flag\", "
				  "\"previous_time\", "
				  "\"time\", "
	              "\"estimated_end_min\", "
	              "\"estimated_end_max\", "
				  "\"category\", "
				  "\"duration_type\", "
				  "\"bid_price\", "
				  "\"price\", "
				  "\"seller\", "
				  "\"bid_flag\", "
				  "\"handle\", "
				  "\"code\", "
				  "\"item_uid\", "
				  "\"count\", "
				  "\"ethereal_durability\", "
				  "\"endurance\", "
				  "\"enhance\", "
				  "\"level\", "
				  "\"flag\", "
				  "\"socket_0\", "
				  "\"socket_1\", "
				  "\"socket_2\", "
				  "\"socket_3\", "
				  "\"awaken_option_value_0\", "
				  "\"awaken_option_value_1\", "
				  "\"awaken_option_value_2\", "
				  "\"awaken_option_value_3\", "
				  "\"awaken_option_value_4\", "
				  "\"awaken_option_data_0\", "
				  "\"awaken_option_data_1\", "
				  "\"awaken_option_data_2\", "
				  "\"awaken_option_data_3\", "
				  "\"awaken_option_data_4\", "
				  "\"remain_time\", "
				  "\"elemental_effect_type\", "
				  "\"elemental_effect_remain_time\", "
				  "\"elemental_effect_attack_point\", "
				  "\"elemental_effect_magic_point\", "
				  "\"appearance_code\") "
	              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
				  DbQueryBinding::EM_NoRow);

	addParam("uid", &InputType::uid);
	addParam("diff_flag", &InputType::diff_flag);
	addParam("previous_time", &InputType::previous_time);
	addParam("time", &InputType::time);
	addParam("estimated_end_min", &InputType::estimatedEndMin);
	addParam("estimated_end_max", &InputType::estimatedEndMax);
	addParam("category", &InputType::category);
	addParam("duration_type", &InputType::duration_type);
	addParam("bid_price", &InputType::bid_price);
	addParam("price", &InputType::price);
	addParam("seller", &InputType::seller);
	addParam("bid_flag", &InputType::bid_flag);
	addParam("handle", &InputType::handle);
	addParam("code", &InputType::code);
	addParam("item_uid", &InputType::item_uid);
	addParam("count", &InputType::count);
	addParam("ethereal_durability", &InputType::ethereal_durability);
	addParam("endurance", &InputType::endurance);
	addParam("enhance", &InputType::enhance);
	addParam("level", &InputType::level);
	addParam("flag", &InputType::flag);
	addParam("socket_0", &InputType::socket, 0);
	addParam("socket_1", &InputType::socket, 1);
	addParam("socket_2", &InputType::socket, 2);
	addParam("socket_3", &InputType::socket, 3);
	addParam("awaken_option_value_0", &InputType::awaken_option_value, 0);
	addParam("awaken_option_value_1", &InputType::awaken_option_value, 1);
	addParam("awaken_option_value_2", &InputType::awaken_option_value, 2);
	addParam("awaken_option_value_3", &InputType::awaken_option_value, 3);
	addParam("awaken_option_value_4", &InputType::awaken_option_value, 4);
	addParam("awaken_option_data_0", &InputType::awaken_option_data, 0);
	addParam("awaken_option_data_1", &InputType::awaken_option_data, 1);
	addParam("awaken_option_data_2", &InputType::awaken_option_data, 2);
	addParam("awaken_option_data_3", &InputType::awaken_option_data, 3);
	addParam("awaken_option_data_4", &InputType::awaken_option_data, 4);
	addParam("remain_time", &InputType::remain_time);
	addParam("elemental_effect_type", &InputType::elemental_effect_type);
	addParam("elemental_effect_remain_time", &InputType::elemental_effect_remain_time);
	addParam("elemental_effect_attack_point", &InputType::elemental_effect_attack_point);
	addParam("elemental_effect_magic_point", &InputType::elemental_effect_magic_point);
	addParam("appearance_code", &InputType::appearance_code);
}
DECLARE_DB_BINDING(DB_Item, "db_item");


