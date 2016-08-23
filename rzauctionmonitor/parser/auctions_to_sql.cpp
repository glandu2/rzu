#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <map>
#include <vector>
#include "Core/CharsetConverter.h"
#include "Database/DbQueryJobRef.h"
#include "Core/Log.h"
#include "Core/EventLoop.h"
#include "Config/GlobalCoreConfig.h"
#include "Database/DbConnectionPool.h"
#include "Database/DbConnection.h"
#include "LibRzuInit.h"
#include "AuctionFile.h"

cval<std::string>& connectionString = CFG_CREATE("connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

struct DB_Item
{
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
		std::string seller;
		int8_t bid_flag;

		uint32_t handle;
		int32_t code;
		int64_t item_uid;
		int64_t count;
		int32_t ethereal_durability;
		uint32_t endurance;
		uint8_t enhance;
		uint8_t level;
		int32_t flag;
		int32_t socket[4];
		uint32_t awaken_option_value[5];
		int32_t awaken_option_data[5];
		int32_t remain_time;
		uint8_t elemental_effect_type;
		int32_t elemental_effect_remain_time;
		int32_t elemental_effect_attack_point;
		int32_t elemental_effect_magic_point;
		int32_t appearance_code;
	};

	struct Output {};
};

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

#pragma pack(push, 1)
struct ItemData {
	uint32_t uid;
	uint32_t handle;
	int32_t code;
	int64_t item_uid;
	int64_t count;
	int32_t ethereal_durability;
	uint32_t endurance;
	uint8_t enhance;
	uint8_t level;
	int32_t flag;
	int32_t socket[4];
	uint32_t awaken_option_value[5];
	int32_t awaken_option_data[5];
	int32_t remain_time;
	uint8_t elemental_effect_type;
	int32_t elemental_effect_remain_time;
	int32_t elemental_effect_attack_point;
	int32_t elemental_effect_magic_point;
	int32_t appearance_code;
	int32_t unknown1[51];
	int16_t unknown2;
};

struct AuctionDataEnd {
	int8_t duration_type;
	int64_t bid_price;
	int64_t price;
	char seller[31];
	int8_t flag;
};

struct AuctionInfo {
	int32_t size;
	int16_t version;
	int16_t flag;
	int64_t time;
	int64_t previousTime;
	int16_t category;
	int32_t uid;
};
#pragma pack(pop)

int main(int argc, char* argv[]) {
	const int totalRecognizedSize = sizeof(struct AuctionInfo) + sizeof(struct ItemData) + sizeof(struct AuctionDataEnd);
	std::map<uint32_t, std::vector<uint8_t>> auctions;

	LibRzuInit();
	DbConnectionPool dbConnectionPool;
	DbBindingLoader::get()->initAll(&dbConnectionPool);

	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
				   GlobalCoreConfig::get()->log.level,
				   GlobalCoreConfig::get()->log.consoleLevel,
				   GlobalCoreConfig::get()->log.dir,
				   GlobalCoreConfig::get()->log.file,
				   GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	ConfigInfo::get()->dump();

	if(argc < 2) {
		Object::logStatic(Object::LL_Info, "main", "Record size is %d(0x%08X)\n", totalRecognizedSize, totalRecognizedSize);
		Object::logStatic(Object::LL_Info, "main", "Usage: %s auctions.bin\n", argv[0]);
		return 0;
	}

	std::vector<DB_Item::Input> inputs;

	size_t processedFiles = 0;
	for(int i = 1; i < argc; i++) {
		const char* filename = argv[i];

		if(filename[0] == '/')
			continue;

		FILE* file = fopen(filename, "rb");
		if(!file) {
			Object::logStatic(Object::LL_Error, "main", "Cant open file %s\n", filename);
			return 1;
		}

		fseek(file, 0, SEEK_END);
		size_t fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* buffer = (char*)malloc(fileSize);
		size_t readDataSize = fread(buffer, 1, fileSize, file);
		if(readDataSize != fileSize) {
			Object::logStatic(Object::LL_Error, "main", "Coulnd't read file data, size: %ld, read: %ld\n", (long int)fileSize, (long int)readDataSize);
			fclose(file);
			free(buffer);
			return 2;
		}
		fclose(file);

		AUCTION_FILE auctionFile;
		AuctionFileHeader* auctionHeader = reinterpret_cast<AuctionFileHeader*>(buffer);
		if(strncmp(auctionHeader->signature, "RAH", 3) != 0) {
			Object::logStatic(Object::LL_Error, "main", "Invalid file, unrecognized header signature\n");
			return 3;
		}
		MessageBuffer structBuffer(buffer, fileSize, auctionHeader->file_version);

		auctionFile.deserialize(&structBuffer);
		if(!structBuffer.checkFinalSize()) {
			Object::logStatic(Object::LL_Error, "main", "Invalid file\n");
			return 3;
		}

		for(size_t i = 0; i < auctionFile.auctions.size(); i++) {
			const AUCTION_INFO& auctionInfo = auctionFile.auctions[i];
			DB_Item::Input input;

			input.uid = auctionInfo.uid;
			input.diff_flag = auctionInfo.diffType;
			input.previous_time.setUnixTime(auctionInfo.previousTime);
			input.time.setUnixTime(auctionInfo.time);
			input.estimatedEndMin.setUnixTime(auctionInfo.estimatedEndTimeMin);
			input.estimatedEndMax.setUnixTime(auctionInfo.estimatedEndTimeMax);
			input.category = auctionInfo.category;

			ItemData* item = (ItemData*) auctionInfo.data.data();
			AuctionDataEnd* dataEnd = (AuctionDataEnd*) (auctionInfo.data.data() + auctionInfo.data.size() - sizeof(AuctionDataEnd));

			input.duration_type = dataEnd->duration_type;
			input.bid_price = dataEnd->bid_price;
			input.price = dataEnd->price;
			input.seller = dataEnd->seller;
			input.bid_flag = dataEnd->flag;

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

			inputs.push_back(input);
		}

		processedFiles++;
	}

	DbQueryJob<DB_Item>::executeNoResult(inputs);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", processedFiles);

	return 0;
}

/*
CREATE TABLE [auctions](
	[uid] [int] NOT NULL,
	[diff_flag] [smallint] NOT NULL,
	[previous_time] [datetime] NOT NULL,
	[time] [datetime] NOT NULL,
	[estimated_end_min] [datetime] NOT NULL,
	[estimated_end_max] [datetime] NOT NULL,
	[category] [smallint] NOT NULL,
	[duration_type] [smallint] NOT NULL,
	[bid_price] [bigint] NOT NULL,
	[price] [bigint] NOT NULL,
	[seller] [varchar](32) NOT NULL,
	[bid_flag] [smallint] NOT NULL,
	[handle] [int] NOT NULL,
	[code] [int] NOT NULL,
	[item_uid] [bigint] NOT NULL,
	[count] [bigint] NOT NULL,
	[ethereal_durability] [int] NOT NULL,
	[endurance] [int] NOT NULL,
	[enhance] [smallint] NOT NULL,
	[level] [smallint] NOT NULL,
	[flag] [int] NOT NULL,
	[socket_0] [int] NOT NULL,
	[socket_1] [int] NOT NULL,
	[socket_2] [int] NOT NULL,
	[socket_3] [int] NOT NULL,
	[awaken_option_value_0] [int] NOT NULL,
	[awaken_option_value_1] [int] NOT NULL,
	[awaken_option_value_2] [int] NOT NULL,
	[awaken_option_value_3] [int] NOT NULL,
	[awaken_option_value_4] [int] NOT NULL,
	[awaken_option_data_0] [int] NOT NULL,
	[awaken_option_data_1] [int] NOT NULL,
	[awaken_option_data_2] [int] NOT NULL,
	[awaken_option_data_3] [int] NOT NULL,
	[awaken_option_data_4] [int] NOT NULL,
	[remain_time] [int] NOT NULL,
	[elemental_effect_type] [smallint] NOT NULL,
	[elemental_effect_remain_time] [int] NOT NULL,
	[elemental_effect_attack_point] [int] NOT NULL,
	[elemental_effect_magic_point] [int] NOT NULL,
	[appearance_code] [int] NOT NULL,
	PRIMARY KEY (
		[uid] ASC,
		[time] ASC
	)
);

*/
