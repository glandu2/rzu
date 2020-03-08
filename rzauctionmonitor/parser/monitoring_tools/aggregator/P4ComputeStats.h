#ifndef P4COMPUTESTATS_H
#define P4COMPUTESTATS_H

#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "P3AggregateStats.h"
#include "PipelineState.h"
#include <stdint.h>

#include "Packet/PacketDeclaration.h"

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
// clang-format on

class P4ComputeStats
    : public PipelineStep<std::pair<PipelineAggregatedState, std::unordered_map<uint32_t, AuctionSummary>>,
                          std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>> {
	DECLARE_CLASSNAME(P4ComputeStats, 0)
public:
	P4ComputeStats();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

private:
	struct DayAggregation {
		struct PriceInfo {
			int64_t itemNumber;
			int64_t minPrice;
			int64_t maxPrice;
			int64_t avgPrice;
			int64_t priceSum;

			PriceInfo() : itemNumber(0), minPrice(-1), maxPrice(-1), avgPrice(-1), priceSum(0) {}
			void updateAggregatedStats(const AuctionSummary& summary);
		};

		PriceInfo allItems;
		PriceInfo soldItems;
	};

private:
	BackgroundWork<P4ComputeStats, std::shared_ptr<WorkItem>> work;
};

#endif
