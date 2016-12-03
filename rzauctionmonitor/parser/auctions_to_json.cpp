#include <stdint.h>
#include <vector>
#include "Core/Log.h"
#include "Config/GlobalCoreConfig.h"
#include "LibRzuInit.h"
#include "AuctionFile.h"
#include "Packet/JSONWriter.h"
#include "AuctionWriter.h"

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
int writeJson(const AUCTION_FILE& auctionFile) {

	JSONWriter jsonWriter(auctionFile.header.file_version, compactJson.get());
	auctionFile.serialize(&jsonWriter);
	jsonWriter.finalize();
	Object::logStatic(Object::LL_Fatal, "main", "%s\n", jsonWriter.toString().c_str());

	return 0;
}

int main(int argc, char* argv[]) {
	LibRzuInit();

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

		std::vector<uint8_t> data;
		int version;
		AuctionFileFormat fileFormat;

		if(!AuctionWriter::readAuctionDataFromFile(filename, data)) {
			Object::logStatic(Object::LL_Error, "main", "Cant read file %s\n", filename);
			return 1;
		}

		if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
			Object::logStatic(Object::LL_Error, "main", "Invalid file, unrecognized header signature: %s\n", filename);
			return 1;
		}

		if(fileFormat == AFF_Complex) {
			AUCTION_FILE file;
			if(!AuctionWriter::deserialize(&file, data)) {
				Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
				return 3;
			}
			writeJson(file);
		} else {
			AUCTION_SIMPLE_FILE file;
			if(!AuctionWriter::deserialize(&file, data)) {
				Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
				return 3;
			}
			writeJson(file);
		}
	}

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", i-1);

	return 0;
}
