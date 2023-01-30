#include "AuctionFile.h"
#include "AuctionWriter.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "LibRzuInit.h"
#include "Packet/JSONWriter.h"
#include <stdint.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>

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
	uint32_t awaken_option_value[7];
	int32_t awaken_option_data[7];
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

#define AUCTION_INFO_PER_DAY_DEF(_) \
	_(simple) \
	(uint32_t, code) _(simple)(uint32_t, estimatedSoldNumber) _(simple)(int64_t, minEstimatedSoldPrice) \
	    _(simple)(int64_t, maxEstimatedSoldPrice) _(simple)(int64_t, avgEstimatedSoldPrice) \
	        _(simple)(uint32_t, itemNumber) _(simple)(int64_t, minPrice) _(simple)(int64_t, maxPrice) \
	            _(simple)(int64_t, avgPrice)
CREATE_STRUCT(AUCTION_INFO_PER_DAY);
#undef AUCTION_INFO_PER_DAY_DEF

#define AUCTION_AGGREGATE_DEF(_) \
	_(simple)(int64_t, date) _(count)(uint32_t, auctionNumber, auctions) _(dynarray)(AUCTION_INFO_PER_DAY, auctions)
CREATE_STRUCT(AUCTION_AGGREGATE);
#undef AUCTION_AGGREGATE_DEF

struct AuctionSummary {
	int64_t price;
	bool isSold;
};

static bool writeToFile(const char* filename, const std::string& data) {
	std::unique_ptr<FILE, int (*)(FILE*)> file(nullptr, &fclose);

	file.reset(fopen(filename, "wt"));
	if(!file) {
		Object::logStatic(Object::LL_Error, "main", "Cannot open aggregation file for write %s\n", filename);
		return false;
	}

	fprintf(file.get(), "%s\n", data.c_str());
	fclose(file.release());

	return true;
}

static void computeStatisticsOfDay(time_t date,
                                   const std::unordered_map<uint32_t, std::vector<AuctionSummary>>& auctionData,
                                   bool compactJson) {
	AUCTION_AGGREGATE aggregateData;
	aggregateData.date = date;

	auto it = auctionData.begin();
	for(; it != auctionData.end(); ++it) {
		AUCTION_INFO_PER_DAY dayAggregation;
		uint32_t code = it->first;
		const std::vector<AuctionSummary>& auctionSummary = it->second;

		size_t i;
		uint32_t num;
		int64_t sum;
		int64_t min;
		int64_t max;

		for(i = 0, num = 0, sum = 0, min = -1, max = -1; i < auctionSummary.size(); i++) {
			int64_t price = auctionSummary[i].price;
			if(auctionSummary[i].isSold == false) {
				if(price < min || min == -1)
					min = price;

				if(price > max || max == -1)
					max = price;

				sum += price;
				num++;
			}
		}

		dayAggregation.code = code;
		dayAggregation.itemNumber = num;
		dayAggregation.minPrice = min;
		dayAggregation.maxPrice = max;
		dayAggregation.avgPrice = num ? sum / num : -1;

		for(i = 0, num = 0, sum = 0, min = -1, max = -1; i < auctionSummary.size(); i++) {
			int64_t price = auctionSummary[i].price;
			if(auctionSummary[i].isSold == true) {
				if(price < min || min == -1)
					min = price;

				if(price > max || max == -1)
					max = price;

				sum += price;
				num++;
			}
		}

		dayAggregation.estimatedSoldNumber = num;
		dayAggregation.minEstimatedSoldPrice = min;
		dayAggregation.maxEstimatedSoldPrice = max;
		dayAggregation.avgEstimatedSoldPrice = num ? sum / num : -1;

		aggregateData.auctions.push_back(dayAggregation);
	}

	JSONWriter jsonWriter(0, compactJson);
	aggregateData.serialize(&jsonWriter);
	jsonWriter.finalize();
	fprintf(stderr, "%s\r\n", jsonWriter.toString().c_str());
}

int main(int argc, char* argv[]) {
	LibRzuInit();

	cval<bool>& compactJson = CFG_CREATE("compactjson", false);
	cval<std::string>& dateString = CFG_CREATE("date", "");

	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
	               GlobalCoreConfig::get()->log.level,
	               GlobalCoreConfig::get()->log.consoleLevel,
	               GlobalCoreConfig::get()->log.dir,
	               GlobalCoreConfig::get()->log.file,
	               GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	if(argc < 2) {
		Object::logStatic(Object::LL_Info, "main", "Usage: %s auctions.bin\n", argv[0]);
		return 0;
	}

	if(dateString.get().size() == 0) {
		Object::logStatic(Object::LL_Info, "main", "/date must be set with format YYYYMMDD\n");
		return 0;
	}

	std::unordered_map<uint32_t, std::vector<AuctionSummary>> auctionData;

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
			return 2;
		}

		AUCTION_FILE auctionFile;
		if(!AuctionWriter::deserialize(&auctionFile, data)) {
			Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
			return 3;
		}

		for(size_t i = 0; i < auctionFile.auctions.size(); i++) {
			const AUCTION_INFO& auctionInfo = auctionFile.auctions[i];
			ItemData* item = (ItemData*) auctionInfo.data.data();

			if((auctionInfo.diffType == D_Added || auctionInfo.diffType == D_Base) && auctionInfo.price) {
				AuctionSummary summary;
				summary.isSold = false;
				summary.price = auctionInfo.price;

				auto it = auctionData.find(item->code);
				if(it == auctionData.end()) {
					auto insertResult = auctionData.insert(std::make_pair(item->code, std::vector<AuctionSummary>()));
					it = insertResult.first;
				}

				it->second.push_back(summary);
			} else if(auctionInfo.diffType == D_Deleted && (auctionInfo.time + 60) < auctionInfo.estimatedEndTimeMin &&
			          auctionInfo.price) {
				AuctionSummary summary;
				summary.isSold = true;
				summary.price = auctionInfo.price;

				auto it = auctionData.find(item->code);
				if(it == auctionData.end()) {
					auto insertResult = auctionData.insert(std::make_pair(item->code, std::vector<AuctionSummary>()));
					it = insertResult.first;
				}

				it->second.push_back(summary);
			}
		}
	}

	computeStatisticsOfDay(auctionData, compactJson);

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", i - 1);

	return 0;
}
