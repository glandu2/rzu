#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "PipelineState.h"
#include <stdint.h>

template<class T> class DbQueryJob;

struct DB_InsertHistory {
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
		int32_t filename_uid;
	};

	struct Output {};

	static void addAuction(std::vector<DB_InsertHistory::Input>& auctions,
	                       int32_t filename_uid,
	                       const AUCTION_INFO& auctionInfo);
	static bool createTable(DbConnectionPool* dbConnectionPool);
};

class P6InsertHistoryToSqlServer
    : public PipelineStep<std::pair<PipelineState, AUCTION_FILE>, std::pair<PipelineState, AUCTION_FILE>> {
	DECLARE_CLASSNAME(P6InsertHistoryToSqlServer, 0)
public:
	P6InsertHistoryToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
	std::vector<DB_InsertHistory::Input> dbInputs;
};
