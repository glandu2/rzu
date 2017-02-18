#include "Aggregator.h"
#include "AuctionFile.h"
#include "Packet/JSONWriter.h"
#include "AuctionWriter.h"
#include "Core/Utils.h"
#include "GlobalConfig.h"
#include "AuctionComplexDiffWriter.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"

#define AUCTION_INFO_PER_DAY_DEF(_) \
	_(simple)(uint32_t, code) \
	_(simple)(int64_t, estimatedSoldNumber) \
	_(simple)(int64_t, minEstimatedSoldPrice) \
	_(simple)(int64_t, maxEstimatedSoldPrice) \
	_(simple)(int64_t, avgEstimatedSoldPrice) \
	_(simple)(int64_t, itemNumber) \
	_(simple)(int64_t, minPrice) \
	_(simple)(int64_t, maxPrice) \
	_(simple)(int64_t, avgPrice)
CREATE_STRUCT(AUCTION_INFO_PER_DAY);

#define AGGREGATION_STATE_VERSION 0

#define AGGREGATION_ITEM_DEF(_) \
	_(simple)(uint32_t, uid) \
	_(simple)(int32_t, itemCode) \
	_(simple)(int64_t, price) \
	_(simple)(bool, isSold) \
	_(simple)(int64_t, count)
CREATE_STRUCT(AGGREGATION_ITEM);

#define AGGREGATION_STATE_DEF(_) \
	_(simple)  (uint16_t, file_version) \
	_(simple)  (uint64_t, currentDate) \
	_(count)   (uint32_t, auctionData) \
	_(dynarray)(AGGREGATION_ITEM, auctionData) \
	_(count)   (uint8_t, lastParsedFile) \
	_(dynstring)(lastParsedFile, false)
CREATE_STRUCT(AGGREGATION_STATE);


Aggregator::Aggregator()
    : httpClientSession(CONFIG_GET()->webserver.ip,
                        CONFIG_GET()->webserver.port),
      url(CONFIG_GET()->webserver.url),
      pwd(CONFIG_GET()->webserver.pwd),
      currentDate(0)
{
}

bool Aggregator::sendToWebServer(const std::string& data)
{
	std::string fullUrl;
	struct tm currentDateTm;

	Utils::getGmTime(currentDate, &currentDateTm);

	Utils::stringFormat(fullUrl, "%s/%04d%02d%02d/%s",
	                    url.get().c_str(),
	                    currentDateTm.tm_year,
	                    currentDateTm.tm_mon,
	                    currentDateTm.tm_mday,
	                    pwd.get().c_str());

	log(LL_Info, "Sending data to url %s\n", fullUrl.c_str());
	httpClientSession.sendData(fullUrl, data);

	return true;
}

void Aggregator::exportState(std::string filename, const std::string& lastParsedFile)
{
	AGGREGATION_STATE aggregationState;

	log(LL_Debug, "Writting aggregation state file %s\n", filename.c_str());

	aggregationState.file_version = AGGREGATION_STATE_VERSION;
	aggregationState.currentDate = currentDate;
	aggregationState.lastParsedFile = lastParsedFile;

	auto it = auctionsByUid.begin();
	auto itEnd = auctionsByUid.end();
	for(; it != itEnd; ++it) {
		const std::pair<uint32_t, AuctionSummary>& val = *it;

		const AuctionSummary& auctionSummary = val.second;
		AGGREGATION_ITEM item;
		item.uid = val.first;
		item.itemCode = auctionSummary.code;
		item.price = auctionSummary.price;
		item.isSold = auctionSummary.isSold;
		item.count = auctionSummary.count;

		aggregationState.auctionData.push_back(item);
	}

	int version = aggregationState.file_version;
	std::vector<uint8_t> data;
	data.resize(aggregationState.getSize(version));

	MessageBuffer buffer(data.data(), data.size(), version);
	aggregationState.serialize(&buffer);
	if(buffer.checkFinalSize() == false) {
		log(LL_Error, "Wrong buffer size, size: %u, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
	} else {
		AuctionWriter::writeAuctionDataToFile(filename, data);
		log(LL_Debug, "Exported %d auctions summaries\n", (int)aggregationState.auctionData.size());
	}
}

void Aggregator::importState(std::string filename, std::string& lastParsedFile)
{
	log(LL_Info, "Loading aggregation state file %s\n", filename.c_str());

	std::vector<uint8_t> data;

	if(AuctionWriter::readAuctionDataFromFile(filename, data)) {
		AGGREGATION_STATE aggregationState;
		uint16_t version = *reinterpret_cast<const uint16_t*>(data.data());
		MessageBuffer buffer(data.data(), data.size(), version);
		aggregationState.deserialize(&buffer);
		if(buffer.checkFinalSize() == false) {
			log(LL_Error, "Wrong buffer size, size: %u, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
			return;
		}

		currentDate = aggregationState.currentDate;
		lastParsedFile = aggregationState.lastParsedFile;

		for(size_t i = 0; i < aggregationState.auctionData.size(); i++) {
			const AGGREGATION_ITEM& item = aggregationState.auctionData[i];
			AuctionSummary auctionSummary;
			auctionSummary.code = item.itemCode;
			auctionSummary.price = item.price;
			auctionSummary.isSold = item.isSold;
			auctionSummary.count = item.count;

			auctionsByUid[item.uid] = auctionSummary;
		}

		log(LL_Debug, "Imported %d auctions summaries\n", (int)aggregationState.auctionData.size());
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
		log(LL_Info, "Date changed from %d to %d, computing statistics of previous day\n", (int)currentDate, (int)date);
		computeStatisticsOfDay();
		currentDate = date;
	} else if(compareResult < 0) {
		log(LL_Warning, "New date is smaller than current date: %d < %d\n", (int)date, (int)currentDate);
	} else {
		log(LL_Debug, "Date from %d to %d not changed\n", (int)currentDate, (int)date);
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

bool Aggregator::parseAuctions(AuctionComplexDiffWriter* auctionWriter) {
	time_t lastestDate = currentDate;

	// don't full dump if we already have auctions
	AUCTION_FILE auctionFile = auctionWriter->exportDump(auctionsByUid.empty() ? true : false, true);

	if(auctionFile.auctions.empty()) {
		log(LL_Debug, "No new auction to parse\n");
		return true;
	}

	if(!auctionFile.header.categories.empty()) {
		lastestDate = auctionFile.header.categories[0].beginTime;
		log(LL_Debug, "Parsing %d auctions for date %d\n", (int)auctionFile.auctions.size(), (int)lastestDate);
	}

	for(size_t i = 0; i < auctionFile.auctions.size(); i++) {
		const AUCTION_INFO& auctionInfo = auctionFile.auctions[i];
		if(!auctionInfo.data.data())
			continue;

		TS_SEARCHED_AUCTION_INFO item;
		MessageBuffer structBuffer(auctionInfo.data.data(), auctionInfo.data.size(), EPIC_LATEST);

		item.deserialize(&structBuffer);
		if(!structBuffer.checkFinalSize()) {
			log(LL_Error, "Invalid item data content for uid %u, can't deserialize\n", auctionInfo.uid);
			return false;
		} else if(structBuffer.getParsedSize() != auctionInfo.data.size()) {
			log(LL_Error, "Invalid item data size for uid %u, can't deserialize safely\n", auctionInfo.uid);
			return false;
		}

		if(skipItem(auctionInfo, item.auction_details.item_info))
			continue;

		AuctionSummary& summary = auctionsByUid[auctionInfo.uid];
		summary.code = item.auction_details.item_info.code;
		summary.enhance = item.auction_details.item_info.enhance;

		if(!summary.isSold) {
			if(item.auction_details.item_info.count > 0)
				summary.count = item.auction_details.item_info.count;
			else
				summary.count = 1;

			summary.price = auctionInfo.bid_price / summary.count;
		}

		if(auctionInfo.diffType == D_Deleted) {
			if(auctionInfo.bid_flag != BF_NoBid) {
				summary.isSold = true;
				summary.price = auctionInfo.bid_price / summary.count;
			}

			if(auctionInfo.price != 0 && auctionInfo.estimatedEndTimeMin != 0 && (auctionInfo.time + 120) < auctionInfo.estimatedEndTimeMin) {
				summary.isSold = true;
				summary.price = auctionInfo.price / summary.count;
			}
		}
	}

	updateCurrentDateAndCompute(lastestDate);

	return true;
}

bool Aggregator::skipItem(const AUCTION_INFO& auctionInfo, const TS_ITEM_BASE_INFO& item)
{
	switch(auctionInfo.category) {
		case C_Weapon:
		case C_Armor:
		case C_Shield:
		case C_Helmet:
		case C_Gloves:
		case C_Boots:
		case C_Cloak:
		case C_Accessory:
			return item.enhance > 0 ||
			        item.awaken_option_value[0] != 0 ||
			        item.awaken_option_value[1] != 0 ||
			        item.awaken_option_value[2] != 0 ||
			        item.awaken_option_value[3] != 0 ||
			        item.awaken_option_value[4] != 0 ||
			        item.random_type[0] != 0 ||
			        item.random_type[1] != 0 ||
			        item.random_type[2] != 0 ||
			        item.random_type[3] != 0 ||
			        item.random_type[4] != 0 ||
			        item.random_type[5] != 0 ||
			        item.random_type[6] != 0 ||
			        item.random_type[7] != 0 ||
			        item.random_type[8] != 0 ||
			        item.random_type[9] != 0;
		case C_Belt:
			return item.enhance > 0 || item.socket[0] > 0;
		case C_SkillCard:
			return item.enhance > 1;
		case C_SummonerCard:
			return (item.flag & 0x80000000) != 0 || item.enhance > 0; // exclude tamed and staged

		default:
			return false;
	}
}


void Aggregator::DayAggregation::PriceInfo::updateAggregatedStats(const Aggregator::AuctionSummary& summary) {
	if(minPrice == -1 || minPrice > summary.price)
		minPrice = summary.price;

	if(maxPrice == -1 || maxPrice < summary.price)
		maxPrice = summary.price;

	avgPrice = (avgPrice * itemNumber + summary.price * summary.count) /
	           (itemNumber + summary.count);

	itemNumber += summary.count;
}

void Aggregator::computeStatisticsOfDay()
{
	struct Key {
		uint32_t code;
		//int16_t enhance;
		Key() {}
		Key(const AuctionSummary& auction) : code(auction.code)/*, enhance(auction.enhance)*/ {}

		bool operator==(const Key& other) const {
			return code == other.code /* && enhance == other.enhance*/;
		}
	};
	struct KeyHasher {
	  std::size_t operator()(const Key& k) const
	  {
		return (std::hash<uint32_t>()(k.code))/* ^ (std::hash<int16_t>()(k.enhance) << 1)) >> 1*/;
	  }
	};

	std::unordered_map<Key, DayAggregation, KeyHasher> auctionsByItemCode;
	JSONWriter jsonWriter(0, true);
	jsonWriter.clear();

	log(LL_Info, "Computing statistics of %d auctions for day %d\n", (int)auctionsByUid.size(), (int)currentDate);

	auto itUid = auctionsByUid.begin();
	for(; itUid != auctionsByUid.end(); ++itUid) {
		const std::pair<uint64_t, AuctionSummary>& val = *itUid;
		const AuctionSummary& summary = val.second;
		Key key(summary);

		DayAggregation& dayAggregation = auctionsByItemCode[key];
		dayAggregation.allItems.updateAggregatedStats(summary);
		if(summary.isSold)
			dayAggregation.soldItems.updateAggregatedStats(summary);
	}

	auto it = auctionsByItemCode.begin();
	for(; it != auctionsByItemCode.end(); ++it) {
		AUCTION_INFO_PER_DAY dayAggregation;
		const Key& key = it->first;
		const DayAggregation& aggregation = it->second;

		dayAggregation.code = key.code;
		//dayAggregation.enhance = key.enhance;

		dayAggregation.itemNumber = aggregation.allItems.itemNumber;
		dayAggregation.minPrice = aggregation.allItems.minPrice;
		dayAggregation.maxPrice = aggregation.allItems.maxPrice;
		dayAggregation.avgPrice = aggregation.allItems.avgPrice;

		dayAggregation.estimatedSoldNumber = aggregation.soldItems.itemNumber;
		dayAggregation.minEstimatedSoldPrice = aggregation.soldItems.minPrice;
		dayAggregation.maxEstimatedSoldPrice = aggregation.soldItems.maxPrice;
		dayAggregation.avgEstimatedSoldPrice = aggregation.soldItems.avgPrice;

		jsonWriter.start();
		dayAggregation.serialize(&jsonWriter);
		jsonWriter.finalize();
	}

	sendToWebServer(jsonWriter.toString());

	auctionsByUid.clear();
}
