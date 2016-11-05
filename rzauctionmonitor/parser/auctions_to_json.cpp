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
#include "Packet/JSONWriter.h"

cval<bool>& compactJson = CFG_CREATE("compactjson", false);

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
#pragma pack(pop)

template<class AUCTION_FILE>
int deserialize(void* buffer, size_t fileSize, int version) {
	AUCTION_FILE auctionFile;

	MessageBuffer structBuffer(buffer, fileSize, version);
	auctionFile.deserialize(&structBuffer);
	if(!structBuffer.checkFinalSize()) {
		Object::logStatic(Object::LL_Error, "main", "Invalid file\n");
		return 3;
	}

	JSONWriter jsonWriter(version, compactJson.get());
	auctionFile.serialize(&jsonWriter);
	jsonWriter.finalize();
	Object::logStatic(Object::LL_Fatal, "main", "%s\n", jsonWriter.toString().c_str());

	return 0;
}

int main(int argc, char* argv[]) {
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
		Object::logStatic(Object::LL_Info, "main", "Usage: %s auctions.bin\n", argv[0]);
		return 0;
	}

	int i;
	for(i = 1; i < argc; i++) {
		const char* filename = argv[i];

		if(filename[0] == '/' || filename[0] == '-')
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
			return 2;
		}
		fclose(file);

		AuctionFileHeader* auctionHeader = reinterpret_cast<AuctionFileHeader*>(buffer);
		if(strncmp(auctionHeader->signature, "RAH", 3) == 0) {
			deserialize<AUCTION_FILE>(buffer, fileSize, auctionHeader->file_version);
		} else if(strncmp(auctionHeader->signature, "RHS", 3) == 0) {
			deserialize<AUCTION_SIMPLE_FILE>(buffer, fileSize, auctionHeader->file_version);
		} else {
			Object::logStatic(Object::LL_Error, "main", "Invalid file, unrecognized header signature\n");
			return 3;
		}
	}

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", i-1);

	return 0;
}

/*
CREATE TABLE "auctions" (
	"uid"	integer NOT NULL,
	"diff_flag"	smallint NOT NULL,
	"previous_time"	INTEGER NOT NULL,
	"time"	INTEGER NOT NULL,
	"category"	smallint NOT NULL,
	"duration_type"	smallint NOT NULL,
	"bid_price"	bigint NOT NULL,
	"price"	bigint NOT NULL,
	"seller"	varchar NOT NULL,
	"bid_flag"	smallint NOT NULL,
	"handle"	integer NOT NULL,
	"code"	integer NOT NULL,
	"item_uid"	bigint NOT NULL,
	"count"	bigint NOT NULL,
	"ethereal_durability"	integer NOT NULL,
	"endurance"	integer NOT NULL,
	"enhance"	smallint NOT NULL,
	"level"	smallint NOT NULL,
	"flag"	integer NOT NULL,
	"socket_0"	integer NOT NULL,
	"socket_1"	integer NOT NULL,
	"socket_2"	integer NOT NULL,
	"socket_3"	integer NOT NULL,
	"awaken_option_value_0"	integer NOT NULL,
	"awaken_option_value_1"	integer NOT NULL,
	"awaken_option_value_2"	integer NOT NULL,
	"awaken_option_value_3"	integer NOT NULL,
	"awaken_option_value_4"	integer NOT NULL,
	"awaken_option_data_0"	integer NOT NULL,
	"awaken_option_data_1"	integer NOT NULL,
	"awaken_option_data_2"	integer NOT NULL,
	"awaken_option_data_3"	integer NOT NULL,
	"awaken_option_data_4"	integer NOT NULL,
	"remain_time"	integer NOT NULL,
	"elemental_effect_type"	smallint NOT NULL,
	"elemental_effect_remain_time"	integer NOT NULL,
	"elemental_effect_attack_point"	integer NOT NULL,
	"elemental_effect_magic_point"	integer NOT NULL,
	"appearance_code"	integer NOT NULL,
	PRIMARY KEY(uid,time)
);
*/
