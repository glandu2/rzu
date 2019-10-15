#include "P4ComputeStats.h"
#include "Packet/JSONWriter.h"

// clang-format off
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

P4ComputeStats::P4ComputeStats()
    : PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE, std::unordered_map<uint32_t, AuctionSummary>>,
                   std::tuple<std::string, time_t, AUCTION_FILE, std::string>,
                   char>(2),
      work(this, &P4ComputeStats::processWork, &P4ComputeStats::afterWork) {}

void P4ComputeStats::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P4ComputeStats::processWork(std::shared_ptr<WorkItem> workItem) {
	const std::tuple<std::string, time_t, AUCTION_FILE, std::unordered_map<uint32_t, AuctionSummary>>& auctionSummary =
	    workItem->getSource();
	const std::unordered_map<uint32_t, AuctionSummary>& auctionsByUid = std::get<3>(auctionSummary);

	struct Key {
		uint32_t code;
		// int16_t enhance;
		Key(const AuctionSummary& auction) : code(auction.code) /*, enhance(auction.enhance)*/ {}

		bool operator==(const Key& other) const { return code == other.code /* && enhance == other.enhance*/; }
	};
	struct KeyHasher {
		std::size_t operator()(const Key& k) const {
			return (std::hash<uint32_t>()(k.code)) /* ^ (std::hash<int16_t>()(k.enhance) << 1)) >> 1*/;
		}
	};

	log(LL_Info,
	    "Computing statistics of %d auctions for day %d\n",
	    (int) auctionsByUid.size(),
	    (int) std::get<1>(auctionSummary));

	std::unordered_map<Key, DayAggregation, KeyHasher> auctionsByItemCode;
	JSONWriter jsonWriter(0, true);
	jsonWriter.clear();

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
		// dayAggregation.enhance = key.enhance;

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

	addResult(workItem,
	          std::make_tuple(std::get<0>(auctionSummary),
	                          std::get<1>(auctionSummary),
	                          std::get<2>(auctionSummary),
	                          jsonWriter.toString()),
	          0);
	return 0;
}

void P4ComputeStats::DayAggregation::PriceInfo::updateAggregatedStats(const AuctionSummary& summary) {
	if(minPrice == -1 || minPrice > summary.price)
		minPrice = summary.price;

	if(maxPrice == -1 || maxPrice < summary.price)
		maxPrice = summary.price;

	avgPrice = (avgPrice * itemNumber + summary.price * summary.count) / (itemNumber + summary.count);

	itemNumber += summary.count;
}

void P4ComputeStats::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
