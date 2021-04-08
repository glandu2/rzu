#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "P4ComputeStats.h"
#include "PipelineState.h"
#include <stdint.h>

struct DB_AuctionSummary;
template<class T> class DbQueryJob;

class P7InsertToSqlServer : public PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                                                PipelineAggregatedState> {
	DECLARE_CLASSNAME(P7InsertToSqlServer, 0)
public:
	P7InsertToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
};
