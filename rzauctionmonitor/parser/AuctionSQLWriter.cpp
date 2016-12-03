#include "AuctionSQLWriter.h"
#include "Database/DbConnectionPool.h"
#include "Database/DbConnection.h"

cval<std::string>& DB_Item::connectionString = CFG_CREATE("connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

template<> void DbQueryJob<DB_Item>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              DB_Item::connectionString,
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

void DB_Item::addAuction(std::vector<DB_Item::Input>& auctions, const AUCTION_INFO& auctionInfo) {
	DB_Item::Input input;

	input.uid = auctionInfo.uid;
	input.diff_flag = auctionInfo.diffType;
	input.previous_time.setUnixTime(auctionInfo.previousTime);
	input.time.setUnixTime(auctionInfo.time);
	input.estimatedEndMin.setUnixTime(auctionInfo.estimatedEndTimeMin);
	input.estimatedEndMax.setUnixTime(auctionInfo.estimatedEndTimeMax);
	input.category = auctionInfo.category;

	ItemData* item = (ItemData*) auctionInfo.data.data();

	input.duration_type = auctionInfo.duration_type;
	input.bid_price = auctionInfo.bid_price;
	input.price = auctionInfo.price;
	input.seller = auctionInfo.seller;
	input.bid_flag = auctionInfo.bid_flag;

	input.handle = item->handle;
	input.code = item->code;
	input.item_uid = item->item_uid;
	input.count = item->count;
	input.ethereal_durability = item->ethereal_durability;
	input.endurance = item->endurance;
	input.enhance = item->enhance;
	input.level = item->level;
	input.flag = item->flag;
	memcpy(input.socket, item->socket, sizeof(input.socket));
	memcpy(input.awaken_option_value, item->awaken_option_value, sizeof(input.awaken_option_value));
	memcpy(input.awaken_option_data, item->awaken_option_data, sizeof(input.awaken_option_data));
	input.remain_time = item->remain_time;
	input.elemental_effect_type = item->elemental_effect_type;
	input.elemental_effect_remain_time = item->elemental_effect_remain_time;
	input.elemental_effect_attack_point = item->elemental_effect_attack_point;
	input.elemental_effect_magic_point = item->elemental_effect_magic_point;
	input.appearance_code = item->appearance_code;

	auctions.push_back(input);
}

void DB_Item::addAuction(std::vector<DB_Item::Input>& auctions, const AUCTION_SIMPLE_INFO& auctionInfo) {
	DB_Item::Input input;

	input.uid = auctionInfo.uid;
	input.diff_flag = auctionInfo.diffType;
	input.previous_time.setUnixTime(auctionInfo.previousTime);
	input.time.setUnixTime(auctionInfo.time);
	input.estimatedEndMin.setUnixTime(0);
	input.estimatedEndMax.setUnixTime(0);
	input.category = auctionInfo.category;

	const ItemData* item = (ItemData*) auctionInfo.data.data();
	if(auctionInfo.data.size() > sizeof(AuctionDataEnd)) {
		const AuctionDataEnd* auctionDataEnd = (const AuctionDataEnd*) (auctionInfo.data.data() + auctionInfo.data.size() - sizeof(AuctionDataEnd));

		input.duration_type = auctionDataEnd->duration_type;
		input.bid_price = auctionDataEnd->bid_price;
		input.price = auctionDataEnd->price;
		input.seller = auctionDataEnd->seller;
		input.bid_flag = auctionDataEnd->bid_flag;
	}

	input.handle = item->handle;
	input.code = item->code;
	input.item_uid = item->item_uid;
	input.count = item->count;
	input.ethereal_durability = item->ethereal_durability;
	input.endurance = item->endurance;
	input.enhance = item->enhance;
	input.level = item->level;
	input.flag = item->flag;
	memcpy(input.socket, item->socket, sizeof(input.socket));
	memcpy(input.awaken_option_value, item->awaken_option_value, sizeof(input.awaken_option_value));
	memcpy(input.awaken_option_data, item->awaken_option_data, sizeof(input.awaken_option_data));
	input.remain_time = item->remain_time;
	input.elemental_effect_type = item->elemental_effect_type;
	input.elemental_effect_remain_time = item->elemental_effect_remain_time;
	input.elemental_effect_attack_point = item->elemental_effect_attack_point;
	input.elemental_effect_magic_point = item->elemental_effect_magic_point;
	input.appearance_code = item->appearance_code;

	auctions.push_back(input);
}

bool DB_Item::createTable(DbConnectionPool* dbConnectionPool)
{
	DbConnection* connection = dbConnectionPool->getConnection(DB_Item::connectionString.get().c_str());
	if(!connection) {
		Object::logStatic(Object::LL_Error, "DB_Item", "Failed to open DB connection\n");
		return false;
	}
	bool createResult = connection->execute(
	              "CREATE TABLE IF NOT EXISTS \"auctions\" (\r\n"
	              "    \"uid\" int NOT NULL,\r\n"
	              "    \"diff_flag\" smallint NOT NULL,\r\n"
	              "    \"previous_time\" datetime NOT NULL,\r\n"
	              "    \"time\" datetime NOT NULL,\r\n"
	              "    \"estimated_end_min\" datetime NOT NULL,\r\n"
	              "    \"estimated_end_max\" datetime NOT NULL,\r\n"
	              "    \"category\" smallint NOT NULL,\r\n"
	              "    \"duration_type\" smallint NOT NULL,\r\n"
	              "    \"bid_price\" bigint NOT NULL,\r\n"
	              "    \"price\" bigint NOT NULL,\r\n"
	              "    \"seller\" varchar(32) NOT NULL,\r\n"
	              "    \"bid_flag\" smallint NOT NULL,\r\n"
	              "    \"handle\" int NOT NULL,\r\n"
	              "    \"code\" int NOT NULL,\r\n"
	              "    \"item_uid\" bigint NOT NULL,\r\n"
	              "    \"count\" bigint NOT NULL,\r\n"
	              "    \"ethereal_durability\" int NOT NULL,\r\n"
	              "    \"endurance\" int NOT NULL,\r\n"
	              "    \"enhance\" smallint NOT NULL,\r\n"
	              "    \"level\" smallint NOT NULL,\r\n"
	              "    \"flag\" int NOT NULL,\r\n"
	              "    \"socket_0\" int NOT NULL,\r\n"
	              "    \"socket_1\" int NOT NULL,\r\n"
	              "    \"socket_2\" int NOT NULL,\r\n"
	              "    \"socket_3\" int NOT NULL,\r\n"
	              "    \"awaken_option_value_0\" int NOT NULL,\r\n"
	              "    \"awaken_option_value_1\" int NOT NULL,\r\n"
	              "    \"awaken_option_value_2\" int NOT NULL,\r\n"
	              "    \"awaken_option_value_3\" int NOT NULL,\r\n"
	              "    \"awaken_option_value_4\" int NOT NULL,\r\n"
	              "    \"awaken_option_data_0\" int NOT NULL,\r\n"
	              "    \"awaken_option_data_1\" int NOT NULL,\r\n"
	              "    \"awaken_option_data_2\" int NOT NULL,\r\n"
	              "    \"awaken_option_data_3\" int NOT NULL,\r\n"
	              "    \"awaken_option_data_4\" int NOT NULL,\r\n"
	              "    \"remain_time\" int NOT NULL,\r\n"
	              "    \"elemental_effect_type\" smallint NOT NULL,\r\n"
	              "    \"elemental_effect_remain_time\" int NOT NULL,\r\n"
	              "    \"elemental_effect_attack_point\" int NOT NULL,\r\n"
	              "    \"elemental_effect_magic_point\" int NOT NULL,\r\n"
	              "    \"appearance_code\" int NOT NULL,\r\n"
	              "    PRIMARY KEY (\r\n"
	              "        \"uid\" ASC,\r\n"
	              "        \"time\" ASC\r\n"
	              "    )\r\n"
	              ");");
	if(!createResult) {
		Object::logStatic(Object::LL_Error, "DB_Item", "Failed to create table \"auctions\"\n");
		return false;
	}

	return true;
}
