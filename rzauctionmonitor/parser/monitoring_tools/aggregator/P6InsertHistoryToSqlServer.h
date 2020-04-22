#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "P4ComputeStats.h"
#include "PipelineState.h"
#include <stdint.h>

template<class T> class DbQueryJob;

class P6InsertHistoryToSqlServer
    : public PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                          std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>> {
	DECLARE_CLASSNAME(P6InsertHistoryToSqlServer, 0)
public:
	P6InsertHistoryToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
};

