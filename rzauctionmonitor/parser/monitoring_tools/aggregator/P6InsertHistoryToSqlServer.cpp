#include "P6InsertHistoryToSqlServer.h"
#include "AuctionWriter.h"
#include "Database/DbConnection.h"
#include "Database/DbConnectionPool.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GlobalConfig.h"
#include "Packet/MessageBuffer.h"
#include <numeric>

typedef int32_t uint32_made_int32_t;
struct DB_InsertItem {
	static cval<std::string>& connectionString;
	struct Input {
		int32_t uid;
		int16_t diff_flag;
		DbDateTime previous_time;
		DbDateTime time;
		DbDateTime estimatedEndMin;
		DbDateTime estimatedEndMax;
		int16_t category;
		int8_t duration_type;
		int64_t bid_price;
		int64_t price;
		DbString<32> seller;
		int8_t bid_flag;

		uint32_made_int32_t handle;
		int32_t code;
		int64_t item_uid;
		int64_t count;
		int32_t ethereal_durability;
		uint32_made_int32_t endurance;
		uint8_t enhance;
		uint8_t level;
		uint16_t enhance_chance;
		uint32_made_int32_t flag;
		int32_t socket[4];
		uint32_made_int32_t awaken_option_value[5];
		int32_t awaken_option_data[5];
		uint32_made_int32_t random_type[10];
		int64_t random_value_1[10];
		int64_t random_value_2[10];
		int32_t remain_time;
		uint8_t elemental_effect_type;
		int32_t elemental_effect_remain_time;
		int32_t elemental_effect_attack_point;
		int32_t elemental_effect_magic_point;
		int32_t appearance_code;
		uint32_made_int32_t summon_code;
		uint32_made_int32_t item_effect_id;
	};

	struct Output {};

	static bool addAuction(std::vector<DB_InsertItem::Input>& auctions, const AUCTION_INFO& auctionInfo);
	static bool createTable(DbConnectionPool* dbConnectionPool);

protected:
	static bool fillItemInfo(Input& input, uint32_t epic, const std::vector<uint8_t>& auctionInfo);
};

cval<std::string>& DB_InsertItem::connectionString =
    CFG_CREATE("db_auctions_data.connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

template<> void DbQueryJob<DB_InsertItem>::init(DbConnectionPool* dbConnectionPool) {
	/*createBinding(dbConnectionPool,
	              DB_InsertItem::connectionString,
	              "INSERT INTO auctions_data ("
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
	              "\"enhance_chance\", "
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
	              "\"random_type_0\", "
	              "\"random_type_1\", "
	              "\"random_type_2\", "
	              "\"random_type_3\", "
	              "\"random_type_4\", "
	              "\"random_type_5\", "
	              "\"random_type_6\", "
	              "\"random_type_7\", "
	              "\"random_type_8\", "
	              "\"random_type_9\", "
	              "\"random_value_1_0\", "
	              "\"random_value_1_1\", "
	              "\"random_value_1_2\", "
	              "\"random_value_1_3\", "
	              "\"random_value_1_4\", "
	              "\"random_value_1_5\", "
	              "\"random_value_1_6\", "
	              "\"random_value_1_7\", "
	              "\"random_value_1_8\", "
	              "\"random_value_1_9\", "
	              "\"random_value_2_0\", "
	              "\"random_value_2_1\", "
	              "\"random_value_2_2\", "
	              "\"random_value_2_3\", "
	              "\"random_value_2_4\", "
	              "\"random_value_2_5\", "
	              "\"random_value_2_6\", "
	              "\"random_value_2_7\", "
	              "\"random_value_2_8\", "
	              "\"random_value_2_9\", "
	              "\"remain_time\", "
	              "\"elemental_effect_type\", "
	              "\"elemental_effect_remain_time\", "
	              "\"elemental_effect_attack_point\", "
	              "\"elemental_effect_magic_point\", "
	              "\"appearance_code\", "
	              "\"summon_code\", "
	              "\"item_effect_id\") "
	              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
	              "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
	              "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
	              DbQueryBinding::EM_NoRow);
*/
	createBinding(dbConnectionPool,
	              DB_InsertItem::connectionString,
	              "{ CALL add_auctions2 (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
	              "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
	              "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) }",
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
	addParam("enhance_chance", &InputType::enhance_chance);
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
	addParam("random_type_0", &InputType::random_type, 0);
	addParam("random_type_1", &InputType::random_type, 1);
	addParam("random_type_2", &InputType::random_type, 2);
	addParam("random_type_3", &InputType::random_type, 3);
	addParam("random_type_4", &InputType::random_type, 4);
	addParam("random_type_5", &InputType::random_type, 5);
	addParam("random_type_6", &InputType::random_type, 6);
	addParam("random_type_7", &InputType::random_type, 7);
	addParam("random_type_8", &InputType::random_type, 8);
	addParam("random_type_9", &InputType::random_type, 9);
	addParam("random_value_1_0", &InputType::random_value_1, 0);
	addParam("random_value_1_1", &InputType::random_value_1, 1);
	addParam("random_value_1_2", &InputType::random_value_1, 2);
	addParam("random_value_1_3", &InputType::random_value_1, 3);
	addParam("random_value_1_4", &InputType::random_value_1, 4);
	addParam("random_value_1_5", &InputType::random_value_1, 5);
	addParam("random_value_1_6", &InputType::random_value_1, 6);
	addParam("random_value_1_7", &InputType::random_value_1, 7);
	addParam("random_value_1_8", &InputType::random_value_1, 8);
	addParam("random_value_1_9", &InputType::random_value_1, 9);
	addParam("random_value_2_0", &InputType::random_value_2, 0);
	addParam("random_value_2_1", &InputType::random_value_2, 1);
	addParam("random_value_2_2", &InputType::random_value_2, 2);
	addParam("random_value_2_3", &InputType::random_value_2, 3);
	addParam("random_value_2_4", &InputType::random_value_2, 4);
	addParam("random_value_2_5", &InputType::random_value_2, 5);
	addParam("random_value_2_6", &InputType::random_value_2, 6);
	addParam("random_value_2_7", &InputType::random_value_2, 7);
	addParam("random_value_2_8", &InputType::random_value_2, 8);
	addParam("random_value_2_9", &InputType::random_value_2, 9);
	addParam("remain_time", &InputType::remain_time);
	addParam("elemental_effect_type", &InputType::elemental_effect_type);
	addParam("elemental_effect_remain_time", &InputType::elemental_effect_remain_time);
	addParam("elemental_effect_attack_point", &InputType::elemental_effect_attack_point);
	addParam("elemental_effect_magic_point", &InputType::elemental_effect_magic_point);
	addParam("appearance_code", &InputType::appearance_code);
	addParam("summon_code", &InputType::summon_code);
	addParam("item_effect_id", &InputType::item_effect_id);
}
DECLARE_DB_BINDING(DB_InsertItem, "db_auctions_data");

bool DB_InsertItem::fillItemInfo(DB_InsertItem::Input& input, uint32_t epic, const std::vector<uint8_t>& data) {
	TS_SEARCHED_AUCTION_INFO item;

	if(data.empty()) {
		Object::logStatic(Object::LL_Warning, "DB_Item", "auctions uid %llu: no item data\n", input.uid);
		return true;
	}

	MessageBuffer structBuffer(data.data(), data.size(), epic);

	item.deserialize(&structBuffer);
	if(!structBuffer.checkFinalSize()) {
		Object::logStatic(Object::LL_Error, "DB_Item", "Invalid item data, can't deserialize\n");
		return false;
	} else {
		if(structBuffer.getParsedSize() != data.size()) {
			Object::logStatic(Object::LL_Warning, "DB_Item", "Invalid item data size, can't deserialize safely\n");
			return false;
		}

		static_assert(sizeof(input.socket) == sizeof(item.auction_details.item_info.socket), "wrong size: socket");
		static_assert(sizeof(input.awaken_option_value) == sizeof(item.auction_details.item_info.awaken_option.value),
		              "wrong size: awaken_option_value");
		static_assert(sizeof(input.awaken_option_data) == sizeof(item.auction_details.item_info.awaken_option.data),
		              "wrong size: awaken_option_data");
		static_assert(sizeof(input.random_type) == sizeof(item.auction_details.item_info.random_type),
		              "wrong size: random_type");
		static_assert(sizeof(input.random_value_1) == sizeof(item.auction_details.item_info.random_value_1),
		              "wrong size: random_value_1");
		static_assert(sizeof(input.random_value_2) == sizeof(item.auction_details.item_info.random_value_2),
		              "wrong size: random_value_2");

		input.handle = item.auction_details.item_info.handle;
		input.code = item.auction_details.item_info.code;
		input.item_uid = item.auction_details.item_info.uid;
		input.count = item.auction_details.item_info.count;
		input.ethereal_durability = item.auction_details.item_info.ethereal_durability;
		input.endurance = item.auction_details.item_info.endurance;
		input.enhance = item.auction_details.item_info.enhance;
		input.level = item.auction_details.item_info.level;
		input.enhance_chance = item.auction_details.item_info.enhance_chance;
		input.flag = item.auction_details.item_info.flag;
		memcpy(input.socket, item.auction_details.item_info.socket, sizeof(input.socket));
		memcpy(input.awaken_option_value,
		       item.auction_details.item_info.awaken_option.value,
		       sizeof(input.awaken_option_value));
		memcpy(input.awaken_option_data,
		       item.auction_details.item_info.awaken_option.data,
		       sizeof(input.awaken_option_data));
		memcpy(input.random_type, item.auction_details.item_info.random_type, sizeof(input.random_type));
		memcpy(input.random_value_1, item.auction_details.item_info.random_value_1, sizeof(input.random_value_1));
		memcpy(input.random_value_2, item.auction_details.item_info.random_value_2, sizeof(input.random_value_2));
		input.remain_time = item.auction_details.item_info.remain_time;
		input.elemental_effect_type = item.auction_details.item_info.elemental_effect_type;
		input.elemental_effect_remain_time = item.auction_details.item_info.elemental_effect_remain_time;
		input.elemental_effect_attack_point = item.auction_details.item_info.elemental_effect_attack_point;
		input.elemental_effect_magic_point = item.auction_details.item_info.elemental_effect_magic_point;
		input.appearance_code = item.auction_details.item_info.appearance_code;
		input.summon_code = item.auction_details.item_info.summon_code;
		input.item_effect_id = item.auction_details.item_info.item_effect_id;
	}

	return true;
}

bool DB_InsertItem::addAuction(std::vector<DB_InsertItem::Input>& auctions, const AUCTION_INFO& auctionInfo) {
	DB_InsertItem::Input input = {};

	input.uid = auctionInfo.uid;
	input.diff_flag = auctionInfo.diffType;
	input.previous_time.setUnixTime(auctionInfo.previousTime);
	input.time.setUnixTime(auctionInfo.time);
	input.estimatedEndMin.setUnixTime(auctionInfo.estimatedEndTimeMin);
	input.estimatedEndMax.setUnixTime(auctionInfo.estimatedEndTimeMax);
	input.category = auctionInfo.category;

	input.duration_type = auctionInfo.duration_type;
	input.bid_price = auctionInfo.bid_price;
	input.price = auctionInfo.price;
	input.seller = auctionInfo.seller;
	input.bid_flag = auctionInfo.bid_flag;

	uint32_t epic = AuctionComplexDiffWriter::parseEpic(auctionInfo.epic, auctionInfo.time);
	if(!fillItemInfo(input, epic, auctionInfo.data)) {
		return false;
	}

	auctions.push_back(input);

	return true;
}

P6InsertHistoryToSqlServer::P6InsertHistoryToSqlServer()
    : PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                   std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>>(3, 3) {}

void P6InsertHistoryToSqlServer::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>> inputData = std::move(item->getSource());
	PipelineAggregatedState& aggregatedState = inputData.first;
	std::vector<DB_InsertItem::Input> dbInputs;
	struct tm currentDay;
	Utils::getGmTime(aggregatedState.base.timestamp, &currentDay);

	item->setName(std::to_string(aggregatedState.base.timestamp));

	dbInputs.reserve(
	    std::accumulate(aggregatedState.dumps.begin(),
	                    aggregatedState.dumps.end(),
	                    (size_t) 0,
	                    [](size_t sum, const AUCTION_FILE& input) { return input.auctions.size() + sum; }));
	for(const auto& auctionsDumps : aggregatedState.dumps) {
		for(const AUCTION_INFO& auctionInfo : auctionsDumps.auctions) {
			// The first dump is a full dump, only put lines that would be in the partial dump to the SQL server
			if(AuctionWriter::diffTypeInPartialDump((DiffType) auctionInfo.diffType)) {
				if(!DB_InsertItem::addAuction(dbInputs, auctionInfo)) {
					log(LL_Error,
					    "%lld: Failed to prepare data for saving in database\n",
					    auctionsDumps.header.categories.back().endTime);
					workDone(item, EBADMSG);
					return;
				}
			}
		}
	}

	log(LL_Info,
	    "Sending %d auctions to SQL table for day %02d/%02d/%04d\n",
	    (int) dbInputs.size(),
	    currentDay.tm_mday,
	    currentDay.tm_mon,
	    currentDay.tm_year);

	aggregatedState.dumps.resize(1);
	aggregatedState.dumps.shrink_to_fit();

	addResult(item, std::move(inputData));

	dbQuery.executeDbQuery<DB_InsertItem>([this, item](auto, int status) { workDone(item, status); }, dbInputs);
}
