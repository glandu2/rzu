#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "P4ComputeStats.h"
#include "PipelineState.h"
#include <stdint.h>

template<class T> class DbQueryJob;

struct DB_InsertHistory {
	static cval<std::string>& connectionString;
	struct Input {
		int32_t uid;
		DbDateTime previous_time;
		DbDateTime time;
		int16_t diff_type;
		uint8_t duration_type;
		int64_t bid_price;
		int8_t bid_flag;
		DbDateTime estimated_end_min;
		DbDateTime estimated_end_max;
	};

	struct Output {};

	static void addAuction(std::vector<DB_InsertHistory::Input>& auctions, const AUCTION_INFO& auctionInfo);
	static bool createTable(DbConnectionPool* dbConnectionPool);
};

class P6InsertHistoryToSqlServer
    : public PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                          std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>> {
	DECLARE_CLASSNAME(P6InsertHistoryToSqlServer, 0)
public:
	P6InsertHistoryToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
	std::vector<DB_InsertHistory::Input> dbInputs;
};
