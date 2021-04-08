#include "P6InsertHistoryToSqlServer.h"
#include "AuctionWriter.h"
#include "Database/DbConnection.h"
#include "Database/DbConnectionPool.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GlobalConfig.h"
#include "Packet/MessageBuffer.h"
#include <array>
#include <numeric>

cval<std::string>& DB_InsertHistory::connectionString =
    CFG_CREATE("db_auctions_history.connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

template<> void DbQueryJob<DB_InsertHistory>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              DB_InsertHistory::connectionString,
	              "INSERT INTO auctions_history ("
	              "\"uid\", "
	              "\"previous_time\", "
	              "\"time\", "
	              "\"diff_type\", "
	              "\"duration_type\", "
	              "\"bid_price\", "
	              "\"bid_flag\", "
	              "\"estimated_end_min\", "
	              "\"estimated_end_max\") "
	              "VALUES\n"
	              "%s\n"
	              "on conflict do nothing;",
	              DbQueryBinding::EM_Insert);

	addParam("uid", &InputType::uid);
	addParam("previous_time", &InputType::previous_time);
	addParam("time", &InputType::time);
	addParam("diff_type", &InputType::diff_type);
	addParam("duration_type", &InputType::duration_type);
	addParam("bid_price", &InputType::bid_price);
	addParam("bid_flag", &InputType::bid_flag);
	addParam("estimated_end_min", &InputType::estimated_end_min);
	addParam("estimated_end_max", &InputType::estimated_end_max);
}
DECLARE_DB_BINDING(DB_InsertHistory, "db_auctions_history");

void DB_InsertHistory::addAuction(std::vector<DB_InsertHistory::Input>& auctions, const AUCTION_INFO& auctionInfo) {
	DB_InsertHistory::Input input = {};

	input.uid = auctionInfo.uid;
	input.previous_time.setUnixTime(auctionInfo.previousTime);
	input.time.setUnixTime(auctionInfo.time);
	input.diff_type = auctionInfo.diffType;
	input.duration_type = auctionInfo.duration_type;
	input.bid_price = auctionInfo.bid_price;
	input.bid_flag = auctionInfo.bid_flag;
	input.estimated_end_min.setUnixTime(auctionInfo.estimatedEndTimeMin);
	input.estimated_end_max.setUnixTime(auctionInfo.estimatedEndTimeMax);

	auctions.push_back(input);
}

P6InsertHistoryToSqlServer::P6InsertHistoryToSqlServer()
    : PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                   std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>>(1, 1) {}

void P6InsertHistoryToSqlServer::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>> inputData = std::move(item->getSource());
	PipelineAggregatedState& aggregatedState = inputData.first;
	struct tm currentDay;
	Utils::getGmTime(aggregatedState.base.timestamp, &currentDay);

	item->setName(std::to_string(aggregatedState.base.timestamp));

	dbInputs.clear();
	dbInputs.reserve(
	    std::accumulate(aggregatedState.dumps.begin(),
	                    aggregatedState.dumps.end(),
	                    (size_t) 0,
	                    [](size_t sum, const AUCTION_FILE& input) { return input.auctions.size() + sum; }));
	for(const auto& auctionsDumps : aggregatedState.dumps) {
		for(const AUCTION_INFO& auctionInfo : auctionsDumps.auctions) {
			// The first dump is a full dump, only put lines that would be in the partial dump to the SQL server
			if(AuctionWriter::diffTypeInPartialDump((DiffType) auctionInfo.diffType)) {
				DB_InsertHistory::addAuction(dbInputs, auctionInfo);
			}
		}
	}

	log(LL_Info,
	    "Sending %d auctions history to SQL table for day %02d/%02d/%04d\n",
	    (int) dbInputs.size(),
	    currentDay.tm_mday,
	    currentDay.tm_mon,
	    currentDay.tm_year);

	aggregatedState.dumps.resize(1);
	aggregatedState.dumps.shrink_to_fit();

	addResult(item, std::move(inputData));

	dbQuery.executeDbQuery<DB_InsertHistory>(
	    [this, item](auto, int status) {
		    dbInputs.clear();
		    workDone(item, status);
	    },
	    dbInputs);
}
