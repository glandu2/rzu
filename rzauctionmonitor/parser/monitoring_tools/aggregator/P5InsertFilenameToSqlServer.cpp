#include "P5InsertFilenameToSqlServer.h"
#include "AuctionWriter.h"
#include "Core/PrintfFormats.h"
#include "Database/DbConnection.h"
#include "Database/DbConnectionPool.h"
#include "GlobalConfig.h"

cval<std::string>& DB_InsertFilename::connectionString =
    CFG_CREATE("db_auctions_filename.connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

template<> void DbQueryJob<DB_InsertFilename>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              DB_InsertFilename::connectionString,
	              "INSERT INTO auctions_filename (\"filename\") "
	              "VALUES\n"
	              "(?)\n"
	              "RETURNING uid;",
	              DbQueryBinding::EM_OneRow);

	addParam("filename", &InputType::filename);

	addColumn("uid", &OutputType::uid);
}
DECLARE_DB_BINDING(DB_InsertFilename, "db_auctions_filename");

P5InsertFilenameToSqlServer::P5InsertFilenameToSqlServer()
    : PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                   std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>>(1, 1) {}

void P5InsertFilenameToSqlServer::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>> inputData = std::move(item->getSource());
	PipelineAggregatedState& aggregatedState = inputData.first;
	struct tm currentDay;
	Utils::getGmTime(aggregatedState.base.timestamp, &currentDay);

	item->setName(std::to_string(aggregatedState.base.timestamp));

	DB_InsertFilename::Input input;
	input.filename = aggregatedState.base.associatedFilename;

	log(LL_Info, "Adding filename %s in SQL database\n", input.filename.c_str());

	dbQuery.executeDbQuery<DB_InsertFilename>(
	    [this, item, input, inputData = std::move(inputData)](auto* query, int status) mutable {
		    if(status == 0) {
			    inputData.first.base.filenameUid = query->getResults().front()->uid;
			    log(LL_Info,
			        "Done adding filename %s with uid %d in SQL database\n",
			        input.filename.c_str(),
			        inputData.first.base.filenameUid);
		    }

		    addResult(item, std::move(inputData));
		    workDone(item, status);
	    },
	    input);
}
