#include "P4ComputeStats.h"
#include "Packet/JSONWriter.h"

struct AuctionByCodeKey {
	uint32_t code;
	// int16_t enhance;
	AuctionByCodeKey(const AuctionSummary& auction) : code(auction.code) /*, enhance(auction.enhance)*/ {}

	bool operator==(const AuctionByCodeKey& other) const { return code == other.code /* && enhance == other.enhance*/; }
};
struct AuctionByCodeKeyHasher {
	std::size_t operator()(const AuctionByCodeKey& k) const {
		return (std::hash<uint32_t>()(k.code)) /* ^ (std::hash<int16_t>()(k.enhance) << 1)) >> 1*/;
	}
};

P4ComputeStats::P4ComputeStats()
    : PipelineStep<std::pair<PipelineAggregatedState, std::unordered_map<uint32_t, AuctionSummary>>,
                   std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>>(0, 1, 1),
      work(this, &P4ComputeStats::processWork, &P4ComputeStats::afterWork) {}

void P4ComputeStats::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P4ComputeStats::processWork(std::shared_ptr<WorkItem> workItem) {
	auto sources = std::move(workItem->getSources());
	for(std::pair<PipelineAggregatedState, std::unordered_map<uint32_t, AuctionSummary>>& input : sources) {
		const std::unordered_map<uint32_t, AuctionSummary>& auctionsByUid = input.second;
		std::vector<AUCTION_INFO_PER_DAY> auctionsInfo;
		struct tm currentDay;

		workItem->setName(std::to_string(input.first.base.timestamp));

		Utils::getGmTime(input.first.base.timestamp, &currentDay);

		log(LL_Info,
		    "Computing statistics of %d auctions for day %02d/%02d/%04d\n",
		    (int) auctionsByUid.size(),
		    currentDay.tm_mday,
		    currentDay.tm_mon,
		    currentDay.tm_year);

		std::unordered_map<AuctionByCodeKey, DayAggregation, AuctionByCodeKeyHasher> auctionsByItemCode;

		auto itUid = auctionsByUid.begin();
		for(; itUid != auctionsByUid.end(); ++itUid) {
			const std::pair<uint64_t, AuctionSummary>& val = *itUid;
			const AuctionSummary& summary = val.second;
			AuctionByCodeKey key(summary);

			DayAggregation& dayAggregation = auctionsByItemCode[key];
			dayAggregation.allItems.updateAggregatedStats(summary);
			if(summary.isSold)
				dayAggregation.soldItems.updateAggregatedStats(summary);
		}

		auctionsInfo.reserve(auctionsByItemCode.size());
		auto it = auctionsByItemCode.begin();
		for(; it != auctionsByItemCode.end(); ++it) {
			AUCTION_INFO_PER_DAY dayAggregation;
			const AuctionByCodeKey& key = it->first;
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

			auctionsInfo.push_back(dayAggregation);
		}

		addResult(workItem, std::make_pair(std::move(input.first), std::move(auctionsInfo)));
	}
	return 0;
}

void P4ComputeStats::DayAggregation::PriceInfo::updateAggregatedStats(const AuctionSummary& summary) {
	if(minPrice == -1 || minPrice > summary.price)
		minPrice = summary.price;

	if(maxPrice == -1 || maxPrice < summary.price)
		maxPrice = summary.price;

	priceSum += summary.price * summary.count;
	itemNumber += summary.count;

	avgPrice = priceSum / itemNumber;
}

void P4ComputeStats::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
