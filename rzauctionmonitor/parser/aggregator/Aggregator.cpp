#include "Aggregator.h"
#include "AuctionFile.h"
#include "Packet/JSONWriter.h"
#include "AuctionWriter.h"
#include "Core/Utils.h"
#include "GlobalConfig.h"

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

#define AUCTION_INFO_PER_DAY_DEF(_) \
	_(simple)(uint32_t, code) \
	_(simple)(uint32_t, estimatedSoldNumber) \
	_(simple)(int64_t, minEstimatedSoldPrice) \
	_(simple)(int64_t, maxEstimatedSoldPrice) \
	_(simple)(int64_t, avgEstimatedSoldPrice) \
	_(simple)(uint32_t, itemNumber) \
	_(simple)(int64_t, minPrice) \
	_(simple)(int64_t, maxPrice) \
	_(simple)(int64_t, avgPrice)
CREATE_STRUCT(AUCTION_INFO_PER_DAY);

#define AGGREGATION_ITEM_DEF(_) \
	_(simple)(uint32_t, itemCode) \
	_(simple)(int64_t, price) \
	_(simple)(bool, isSold)
CREATE_STRUCT(AGGREGATION_ITEM);

#define AGGREGATION_STATE_DEF(_) \
	_(simple)  (uint64_t, currentDate) \
	_(count)   (uint32_t, auctionNumber, auctionData) \
	_(dynarray)(AGGREGATION_ITEM, auctionData) \
	_(count)   (uint8_t, lastParsedFileSize, lastParsedFile) \
	_(dynstring)(lastParsedFile, false)
CREATE_STRUCT(AGGREGATION_STATE);


Aggregator::Aggregator()
    : httpClientSession(CONFIG_GET()->webserver.ip,
                        CONFIG_GET()->webserver.port,
                        CONFIG_GET()->webserver.hostname),
      currentDate(0),
      url(CONFIG_GET()->webserver.url)
{
}

bool Aggregator::writeToFile(const char* filename, const std::string& data)
{
	std::unique_ptr<FILE, int(*)(FILE*)> file(nullptr, &fclose);

	file.reset(fopen(filename, "wt"));
	if(!file) {
		Object::logStatic(Object::LL_Error, "main", "Cannot open aggregation file for write %s\n", filename);
		return false;
	}

	fprintf(file.get(), "%s\n", data.c_str());
	fclose(file.release());

	return true;
}

bool Aggregator::sendToWebServer(const std::string& data)
{
	std::string fullUrl;
	struct tm currentDateTm;

	Utils::getGmTime(currentDate, &currentDateTm);

	Utils::stringFormat(fullUrl, "%s/%04d%02d%02d",
	                    url.get().c_str(),
	                    currentDateTm.tm_year,
	                    currentDateTm.tm_mon,
	                    currentDateTm.tm_mday);

	log(LL_Debug, "Sending data to url %s\n", fullUrl.c_str());
	httpClientSession.sendData(fullUrl, data);

	return true;
}

void Aggregator::exportState(std::string filename, const std::string& lastParsedFile)
{
	AGGREGATION_STATE aggregationState;

	log(LL_Info, "Writting aggregation state file %s\n", filename.c_str());

	aggregationState.currentDate = currentDate;
	aggregationState.lastParsedFile = lastParsedFile;

	auto it = auctionData.begin();
	auto itEnd = auctionData.end();
	for(; it != itEnd; ++it) {
		const std::pair<uint32_t, std::vector<AuctionSummary>>& val = *it;

		for(size_t i = 0; i < val.second.size(); i++) {
			const AuctionSummary& auctionSummary = val.second[i];
			AGGREGATION_ITEM item;
			item.itemCode = val.first;
			item.price = auctionSummary.price;
			item.isSold = auctionSummary.isSold;

			aggregationState.auctionData.push_back(item);
		}
	}

	int version = 0;
	std::vector<uint8_t> data;
	data.resize(aggregationState.getSize(version));

	MessageBuffer buffer(data.data(), data.size(), version);
	aggregationState.serialize(&buffer);
	if(buffer.checkFinalSize() == false) {
		log(LL_Error, "Wrong buffer size, size: %d, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
	} else {
		AuctionWriter::writeAuctionDataToFile(filename, data);
		log(LL_Debug, "Exported %d auctions summaries\n", aggregationState.auctionData.size());
	}
}

void Aggregator::importState(std::string filename, std::string& lastParsedFile)
{
	log(LL_Info, "Loading aggregation state file %s\n", filename.c_str());

	std::vector<uint8_t> data;

	if(AuctionWriter::readAuctionDataFromFile(filename, data)) {
		AGGREGATION_STATE aggregationState;
		MessageBuffer buffer(data.data(), data.size(), 0);
		aggregationState.deserialize(&buffer);
		if(buffer.checkFinalSize() == false) {
			log(LL_Error, "Wrong buffer size, size: %d, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
			return;
		}

		currentDate = aggregationState.currentDate;
		lastParsedFile = aggregationState.lastParsedFile;

		for(size_t i = 0; i < aggregationState.auctionData.size(); i++) {
			const AGGREGATION_ITEM& item = aggregationState.auctionData[i];
			AuctionSummary auctionSummary;
			auctionSummary.price = item.price;
			auctionSummary.isSold = item.isSold;

			auctionData[item.itemCode].push_back(auctionSummary);
		}

		log(LL_Debug, "Imported %d auctions summaries\n", aggregationState.auctionData.size());
	} else {
		log(LL_Warning, "Cant read state file %s\n", filename.c_str());
	}

}

void Aggregator::updateCurrentDateAndCompute(time_t date)
{
	int compareResult = compareWithCurrentDate(date);
	if(currentDate == 0) {
		currentDate = date;
	} else if(compareResult > 0) {
		log(LL_Info, "Date changed from %d to %d, computing statistics of previous day\n", currentDate, date);
		computeStatisticsOfDay();
		currentDate = date;
	} else if(compareResult < 0) {
		log(LL_Warning, "New date is smaller than current date: %d < %d\n", date, currentDate);
	} else {
		log(LL_Debug, "Date from %d to %d not changed\n", currentDate, date);
	}
}

int Aggregator::compareWithCurrentDate(time_t other)
{
	struct tm currentDateTm;
	struct tm otherDateTm;

	Utils::getGmTime(currentDate, &currentDateTm);
	Utils::getGmTime(other, &otherDateTm);

	log(LL_Debug, "Date changed from %04d-%02d-%02d to %04d-%02d-%02d\n",
	    currentDateTm.tm_year,
	    currentDateTm.tm_mon,
	    currentDateTm.tm_mday,
	    otherDateTm.tm_year,
	    otherDateTm.tm_mon,
	    otherDateTm.tm_mday);

	if(otherDateTm.tm_year != currentDateTm.tm_year)
		return otherDateTm.tm_year - currentDateTm.tm_year;

	if(otherDateTm.tm_mon != currentDateTm.tm_mon)
		return otherDateTm.tm_mon - currentDateTm.tm_mon;

	return otherDateTm.tm_mday - currentDateTm.tm_mday;
}

void Aggregator::computeStatisticsOfDay()
{
	JSONWriter jsonWriter(0, true);
	jsonWriter.clear();

	log(LL_Info, "Computing statistics of %d items for day %d\n", auctionData.size(), currentDate);

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
		dayAggregation.avgPrice = num ? sum/num : -1;

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
		dayAggregation.avgEstimatedSoldPrice = num ? sum/num : -1;

		jsonWriter.start();
		dayAggregation.serialize(&jsonWriter);
		jsonWriter.finalize();
	}

	sendToWebServer(jsonWriter.toString());

	auctionData.clear();
}

bool Aggregator::parseFile(const char* filename) {
	std::vector<uint8_t> data;
	int version;
	AuctionFileFormat fileFormat;

	log(LL_Info, "Parsing file %s\n", filename);

	if(!AuctionWriter::readAuctionDataFromFile(filename, data)) {
		Object::logStatic(Object::LL_Error, "main", "Cant read file %s\n", filename);
		return false;
	}

	if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
		Object::logStatic(Object::LL_Error, "main", "Invalid file, unrecognized header signature: %s\n", filename);
		return false;
	}

	AUCTION_FILE auctionFile;
	if(!AuctionWriter::deserialize(&auctionFile, data)) {
		Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
		return false;
	}

	parseAuctions(auctionFile);

	return true;
}

time_t Aggregator::parseAuctions(const AUCTION_FILE& auctionFile) {
	time_t lastestDate = currentDate;

	if(auctionFile.auctions.empty()) {
		log(LL_Info, "No auction to parse in file\n");
		return currentDate;
	}

	if(!auctionFile.header.categories.empty()) {
		lastestDate = auctionFile.header.categories[0].beginTime;
		log(LL_Info, "Parsing %d auctions for date %d\n", auctionFile.auctions.size(), lastestDate);
	}

	for(size_t i = 0; i < auctionFile.auctions.size(); i++) {
		const AUCTION_INFO& auctionInfo = auctionFile.auctions[i];
		ItemData* item = (ItemData*) auctionInfo.data.data();

		if((auctionInfo.diffType == D_Added || auctionInfo.diffType == D_Base) && auctionInfo.price) {
			AuctionSummary summary;
			summary.isSold = false;
			summary.price = auctionInfo.price;

			auctionData[item->code].push_back(summary);
		} else if(auctionInfo.diffType == D_Deleted && (auctionInfo.time + 60) < auctionInfo.estimatedEndTimeMin && auctionInfo.price) {
			AuctionSummary summary;
			summary.isSold = true;
			summary.price = auctionInfo.price;

			auctionData[item->code].push_back(summary);
		}
	}

	updateCurrentDateAndCompute(lastestDate);

	return lastestDate;
}
