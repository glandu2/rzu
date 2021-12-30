#include "P5InsertFilenameToSqlServer.h"
#include "AuctionWriter.h"
#include "Core/PrintfFormats.h"
#include "Database/DbConnection.h"
#include "Database/DbConnectionPool.h"
#include "GlobalConfig.h"

template<> void DbQueryJob<DB_InsertFilename>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->db.connectionString,
	              "WITH e AS (\n"
	              "    INSERT INTO auctions_filename (\"filename\", \"start_time\") VALUES\n"
	              "        (?, ?)\n"
	              "        on conflict(\"filename\") do nothing\n"
	              "        RETURNING uid\n"
	              ")\n"
	              "SELECT * FROM e\n"
	              "UNION\n"
	              "    SELECT uid FROM auctions_filename WHERE filename = ?;\n",
	              DbQueryBinding::EM_OneRow);

	addParam("filename", &InputType::filename);
	addParam("start_time", &InputType::start_time);
	addParam("filename", &InputType::filename);

	addColumn("uid", &OutputType::uid);
}
DECLARE_DB_BINDING(DB_InsertFilename, "db_auctions_filename");

P5InsertFilenameToSqlServer::P5InsertFilenameToSqlServer()
    : PipelineStep<std::pair<PipelineState, AUCTION_FILE>, std::pair<PipelineState, AUCTION_FILE>>(1, 1) {}

void P5InsertFilenameToSqlServer::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	std::pair<PipelineState, AUCTION_FILE> inputData = std::move(item->getSource());
	PipelineState& aggregatedState = inputData.first;
	struct tm currentDay;
	Utils::getGmTime(aggregatedState.timestamp, &currentDay);

	item->setName(std::to_string(aggregatedState.timestamp));

	DB_InsertFilename::Input input;
	input.filename = aggregatedState.associatedFilename;
	input.start_time.setUnixTime(aggregatedState.timestamp);

	log(LL_Info, "Adding filename %s in SQL database\n", input.filename.c_str());

	dbQuery.executeDbQuery<DB_InsertFilename>(
	    [this, item, input, inputData = std::move(inputData)](auto* query, int status) mutable {
		    if(status == 0) {
			    if(query->getResults().empty()) {
				    log(LL_Error,
				        "No new filename inserted for %s, maybe already existing ?, skipping related auctions\n",
				        inputData.first.associatedFilename.c_str());
			    } else {
				    inputData.first.filenameUid = query->getResults().front()->uid;
				    log(LL_Info,
				        "Done adding filename %s with uid %d in SQL database\n",
				        input.filename.c_str(),
				        inputData.first.filenameUid);

				    addResult(item, std::move(inputData));
			    }
		    }
		    workDone(item, status);
	    },
	    input);
}
