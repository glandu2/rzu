#ifndef P4COMPUTESTATS_H
#define P4COMPUTESTATS_H

#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "P3AggregateStats.h"
#include <stdint.h>

class P4ComputeStats
    : public PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE, std::unordered_map<uint32_t, AuctionSummary>>,
                          std::tuple<std::string, time_t, AUCTION_FILE, std::string>,
                          char>,
      public Object {
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

			PriceInfo() : itemNumber(0), minPrice(-1), maxPrice(-1), avgPrice(-1) {}
			void updateAggregatedStats(const AuctionSummary& summary);
		};

		PriceInfo allItems;
		PriceInfo soldItems;
	};

private:
	BackgroundWork<P4ComputeStats, std::shared_ptr<WorkItem>> work;
};

#endif
