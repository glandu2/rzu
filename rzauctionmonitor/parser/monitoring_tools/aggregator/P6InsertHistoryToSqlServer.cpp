#include "P6InsertHistoryToSqlServer.h"
#include "AuctionWriter.h"
#include "Database/DbConnection.h"
#include "Database/DbConnectionPool.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GlobalConfig.h"
#include "Packet/MessageBuffer.h"
#include <array>
#include <numeric>

template<> void DbQueryJob<DB_InsertHistory>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->db.connectionString,
	              "INSERT INTO auctions_history ("
	              "\"uid\", "
	              "\"previous_time\", "
	              "\"time\", "
	              "\"diff_type\", "
	              "\"duration_type\", "
	              "\"bid_price\", "
	              "\"bid_flag\", "
	              "\"estimated_end_min\", "
	              "\"estimated_end_max\", "
	              "\"filename_uid\") "
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
	addParam("filename_uid", &InputType::filename_uid);
}
DECLARE_DB_BINDING(DB_InsertHistory, "db_auctions_history");

void DB_InsertHistory::addAuction(std::vector<DB_InsertHistory::Input>& auctions,
                                  int32_t filename_uid,
                                  const AUCTION_INFO& auctionInfo) {
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
	input.filename_uid = filename_uid;

	auctions.push_back(input);
}

P6InsertHistoryToSqlServer::P6InsertHistoryToSqlServer()
    : PipelineStep<std::pair<PipelineState, AUCTION_FILE>, std::pair<PipelineState, AUCTION_FILE>>(500, 1, 100) {}

void P6InsertHistoryToSqlServer::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	auto sources = std::move(item->getSources());

	dbInputs.clear();
	dbInputs.reserve(std::accumulate(sources.begin(),
	                                 sources.end(),
	                                 (size_t) 0,
	                                 [](size_t sum, const std::pair<PipelineState, AUCTION_FILE>& input) {
		                                 return input.second.auctions.size() + sum;
	                                 }));

	for(std::pair<PipelineState, AUCTION_FILE>& inputData : sources) {
		PipelineState& aggregatedState = inputData.first;

		item->setName(std::to_string(aggregatedState.timestamp));
		for(AUCTION_INFO& auctionInfo : inputData.second.auctions) {
			// The first dump is a full dump, only put lines that would be in the partial dump to the SQL server
			if(AuctionWriter::diffTypeInPartialDump((DiffType) auctionInfo.diffType)) {
				DB_InsertHistory::addAuction(dbInputs, aggregatedState.filenameUid, auctionInfo);
			}
		}
		addResult(item, std::move(inputData));
	}

	log(LL_Info, "Sending %d auctions history to SQL table\n", (int) dbInputs.size());

	dbQuery.executeDbQuery<DB_InsertHistory>(
	    [this, item](auto, int status) {
		    log(LL_Info, "Done Sending %d auctions history to SQL table\n", (int) dbInputs.size());
		    dbInputs.clear();
		    workDone(item, status);
	    },
	    dbInputs);
}
