#include "P0LoadActiveAuctions.h"
#include "AuctionFile.h"
#include "AuctionWriter.h"
#include "Database/DbQueryJob.h"
#include "GlobalConfig.h"
#include "Packet/MessageBuffer.h"

template<> void DbQueryJob<DB_NextAuctionFileToParse>::init(DbConnectionPool* dbConnectionPool) {
	// Read the 4th latest filename to correctly handle "maybeDeleted" state
	createBinding(dbConnectionPool,
	              CONFIG_GET()->db.connectionString,
	              "select "
	              "    uid, "
	              "    filename, "
	              "    start_time "
	              "from auctions_filename "
	              "order by uid desc "
	              "offset 3 "
	              "limit 1 ",
	              DbQueryBinding::EM_OneRow);

	addColumn("uid", &OutputType::uid);
	addColumn("filename", &OutputType::filename);
	addColumn("start_time", &OutputType::start_time);
}
DECLARE_DB_BINDING(DB_NextAuctionFileToParse, "db_auctions_lastfile");

template<> void DbQueryJob<DB_ActiveAuctions>::init(DbConnectionPool* dbConnectionPool) {
	// We do + 3 for filename_uid as deleted auctions are removed only after 4 times absent
	createBinding(dbConnectionPool,
	              CONFIG_GET()->db.connectionString,
	              "select\n"
	              "    ad.uid,\n"
	              "    ah.previous_time,\n"
	              "    ah.time,\n"
	              "    ah.diff_type,\n"
	              "    ah.estimated_end_min,\n"
	              "    ah.estimated_end_max,\n"
	              "    ad.category,\n"
	              "    ah.duration_type,\n"
	              "    ah.bid_price,\n"
	              "    ad.price,\n"
	              "    ad.seller,\n"
	              "    ah.bid_flag\n"
	              "from auctions_data ad\n"
	              "cross join lateral (\n"
	              "    select *\n"
	              "    from auctions_history ah2\n"
	              "    where ah2.uid = ad.uid and (\n"
	              "       (diff_type = 2 and filename_uid <= ? + 3) OR\n"
	              "       (diff_type in (0, 1) and filename_uid <= ?)\n"
	              "    )\n"
	              "    order by uid desc, time desc\n"
	              "    limit 1\n"
	              ") ah\n"
	              "where\n"
	              "    ad.created_time_max >= (?::timestamp - interval '10' day) AND\n"
	              "    ah.diff_type BETWEEN 0 AND 1\n",
	              DbQueryBinding::EM_MultiRows);

	addParam("filename_uid", &InputType::uid);
	addParam("filename_uid", &InputType::uid);
	addParam("file_start_time", &InputType::start_time);

	addColumn("uid", &OutputType::uid);
	addColumn("previous_time", &OutputType::previous_time);
	addColumn("time", &OutputType::time);
	addColumn("estimated_end_min", &OutputType::estimated_end_min);
	addColumn("estimated_end_max", &OutputType::estimated_end_max);
	addColumn("category", &OutputType::category);
	addColumn("duration_type", &OutputType::duration_type);
	addColumn("bid_price", &OutputType::bid_price);
	addColumn("price", &OutputType::price);
	addColumn("seller", &OutputType::seller);
	addColumn("bid_flag", &OutputType::bid_flag);
}
DECLARE_DB_BINDING(DB_ActiveAuctions, "db_auctions_actives");

P0LoadActiveAuctions::P0LoadActiveAuctions() {}

void P0LoadActiveAuctions::load(LoadCallback callback) {
	this->callback = callback;
	loadLastFileName();
}

void P0LoadActiveAuctions::loadLastFileName() {
	log(LL_Info, "Reading latest parsed file from database\n");

	dbQuery.executeDbQuery<DB_NextAuctionFileToParse>([this](DbQueryJob<DB_NextAuctionFileToParse>* queryJob,
	                                                         int status) { doneLoadingLastFilename(queryJob, status); },
	                                                  DB_NextAuctionFileToParse::Input{});
}

void P0LoadActiveAuctions::doneLoadingLastFilename(DbQueryJob<DB_NextAuctionFileToParse>* queryJob, int status) {
	if(status) {
		log(LL_Error, "Failed to read database: %d\n", status);
		callback(false, status, nullptr, "");
		return;
	}

	if(queryJob->getResults().empty()) {
		log(LL_Info, "No data in database, starting reading files from the beginning\n");
		callback(true, status, nullptr, "");
		return;
	}

	const LastParsedFileName* lastParsedFile = queryJob->getResults().front().get();
	log(LL_Info, "Latest file is %s with uid %d\n", lastParsedFile->filename.c_str(), lastParsedFile->uid);

	lastAuctionFilename = lastParsedFile->filename;

	log(LL_Info, "Loading active auctions from database\n");

	dbQuery.executeDbQuery<DB_ActiveAuctions>(
	    [this](DbQueryJob<DB_ActiveAuctions>* queryJob, int status) { doneLoadingAuctions(queryJob, status); },
	    *lastParsedFile);
}

void P0LoadActiveAuctions::doneLoadingAuctions(DbQueryJob<DB_ActiveAuctions>* queryJob, int status) {
	if(status) {
		log(LL_Error, "Failed to loaded active auctions: %d\n", status);
		return;
	}

	AUCTION_FILE auctionData;
	if(importAuctions(lastAuctionFilename, queryJob, &auctionData)) {
		callback(true, 0, &auctionData, lastAuctionFilename);
	} else {
		callback(false, ENOENT, nullptr, "");
	}
}

bool P0LoadActiveAuctions::importAuctions(const std::string& lastParsedFile,
                                          DbQueryJob<DB_ActiveAuctions>* queryJob,
                                          AUCTION_FILE* auctionData) {
	std::vector<AuctionWriter::file_data_byte> data;
	AUCTION_SIMPLE_FILE auctionsFile;
	const std::vector<std::unique_ptr<DB_ActiveAuctions::Output> >& results = queryJob->getResults();

	log(LL_Info, "Loaded %d active auctions\n", (int) results.size());

	std::string lastAuctionDataFile = CONFIG_GET()->input.auctionsPath.get() + "/" + lastParsedFile;

	log(LL_Info, "Loading auction category times from file %s\n", lastAuctionDataFile.c_str());

	if(!AuctionWriter::readAuctionDataFromFile(lastAuctionDataFile, data)) {
		log(LL_Error, "File %s cannot be read\n", lastAuctionDataFile.c_str());
		return false;
	}

	if(data.size() < 2) {
		log(LL_Error, "Auction state file %s is too small\n", lastAuctionDataFile.c_str());
		return false;
	}

	uint16_t version = *reinterpret_cast<const uint16_t*>(data.data());
	MessageBuffer buffer(data.data(), data.size(), version);
	auctionsFile.deserialize(&buffer);
	if(buffer.checkFinalSize() == false) {
		log(LL_Error, "Can't deserialize state file %s\n", lastAuctionDataFile.c_str());
		log(LL_Error,
		    "Wrong buffer size, size: %u, field: %s\n",
		    buffer.getSize(),
		    buffer.getFieldInOverflow().c_str());
		return false;
	}

	log(LL_Info,
	    "Loaded %d category times state file %s\n",
	    (int) auctionsFile.header.categories.size(),
	    lastAuctionDataFile.c_str());

	auctionData->header = auctionsFile.header;

	auctionData->header.file_version = AUCTION_LATEST;
	auctionData->header.dumpType = DT_Full;

	auctionData->auctions.reserve(results.size());
	for(const auto& item : results) {
		AUCTION_INFO auction;

		auction.uid = item->uid;
		auction.time = item->time.getUnixTime();
		auction.previousTime = item->previous_time.getUnixTime();
		auction.estimatedEndTimeFromAdded = true;
		auction.estimatedEndTimeMin = item->estimated_end_min.getUnixTime();
		auction.estimatedEndTimeMax = item->estimated_end_max.getUnixTime();
		auction.diffType = D_Unmodified;
		auction.category = item->category;
		auction.epic = EPIC_LATEST;
		auction.duration_type = item->duration_type;
		auction.bid_price = item->bid_price;
		auction.price = item->price;
		auction.seller = item->seller;
		auction.bid_flag = item->bid_flag;
		auction.deleted = false;
		auction.deletedCount = 0;

		auctionData->auctions.push_back(std::move(auction));
	}

	return true;
}
