#include "P5InsertToSqlServer.h"
#include "GlobalConfig.h"

struct DB_AuctionSummary {
	static cval<std::string>& connectionString;
	struct Input {
		DbDateTime day;
		uint32_t code;
		int64_t estimatedSoldNumber;
		int64_t minEstimatedSoldPrice;
		int64_t maxEstimatedSoldPrice;
		int64_t avgEstimatedSoldPrice;
		SQLLEN minEstimatedSoldPriceNullIndicator;
		SQLLEN maxEstimatedSoldPriceNullIndicator;
		SQLLEN avgEstimatedSoldPriceNullIndicator;
		int64_t itemNumber;
		int64_t minPrice;
		int64_t maxPrice;
		int64_t avgPrice;
		SQLLEN minPriceNullIndicator;
		SQLLEN maxPriceNullIndicator;
		SQLLEN avgPriceNullIndicator;
	};
	struct Output {};
};

cval<std::string>& DB_AuctionSummary::connectionString =
    CFG_CREATE("connectionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

template<> void DbQueryJob<DB_AuctionSummary>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              DB_AuctionSummary::connectionString,
	              "INSERT INTO auctions ("
	              "day, "
	              "code, "
	              "estimatedSoldNumber, "
	              "minEstimatedSoldPrice, "
	              "maxEstimatedSoldPrice, "
	              "avgEstimatedSoldPrice, "
	              "itemNumber, "
	              "minPrice, "
	              "maxPrice, "
	              "avgPrice) "

	              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
	              DbQueryBinding::EM_NoRow);

	addParam("day", &InputType::day);
	addParam("code", &InputType::code);
	addParam("estimatedSoldNumber", &InputType::estimatedSoldNumber);
	addParam(
	    "minEstimatedSoldPrice", &InputType::minEstimatedSoldPrice, &InputType::minEstimatedSoldPriceNullIndicator);
	addParam(
	    "maxEstimatedSoldPrice", &InputType::maxEstimatedSoldPrice, &InputType::maxEstimatedSoldPriceNullIndicator);
	addParam(
	    "avgEstimatedSoldPrice", &InputType::avgEstimatedSoldPrice, &InputType::avgEstimatedSoldPriceNullIndicator);
	addParam("itemNumber", &InputType::itemNumber);
	addParam("minPrice", &InputType::minPrice, &InputType::minPriceNullIndicator);
	addParam("maxPrice", &InputType::maxPrice, &InputType::maxPriceNullIndicator);
	addParam("avgPrice", &InputType::avgPrice, &InputType::avgPriceNullIndicator);
}
DECLARE_DB_BINDING(DB_AuctionSummary, "db_auctions");

P5InsertToSqlServer::P5InsertToSqlServer()
    : PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE, std::vector<AUCTION_INFO_PER_DAY>>,
                   std::tuple<std::string, time_t, AUCTION_FILE>>(10, 2) {}

void P5InsertToSqlServer::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	std::tuple<std::string, time_t, AUCTION_FILE, std::vector<AUCTION_INFO_PER_DAY>> inputData =
	    std::move(item->getSource());

	time_t currentDayTime = std::get<1>(inputData);
	const std::vector<AUCTION_INFO_PER_DAY>& inputAuctions = std::get<3>(inputData);
	std::vector<DB_AuctionSummary::Input> dataToWrite;
	struct tm currentDay;
	Utils::getGmTime(currentDayTime, &currentDay);

	item->setName(std::to_string(currentDayTime));
	log(LL_Info,
	    "Sending data to SQL table for day %02d/%02d/%04d\n",
	    currentDay.tm_mday,
	    currentDay.tm_mon,
	    currentDay.tm_year);

	dataToWrite.reserve(inputAuctions.size());
	for(const AUCTION_INFO_PER_DAY& auctionInfo : inputAuctions) {
		DB_AuctionSummary::Input data;

		data.day.setUnixTime(currentDayTime);
		data.code = auctionInfo.code;
		data.estimatedSoldNumber = auctionInfo.estimatedSoldNumber;
		data.minEstimatedSoldPrice = auctionInfo.minEstimatedSoldPrice;
		data.maxEstimatedSoldPrice = auctionInfo.maxEstimatedSoldPrice;
		data.avgEstimatedSoldPrice = auctionInfo.avgEstimatedSoldPrice;
		data.minEstimatedSoldPriceNullIndicator = auctionInfo.minEstimatedSoldPrice == -1 ? SQL_NULL_DATA : 0;
		data.maxEstimatedSoldPriceNullIndicator = auctionInfo.maxEstimatedSoldPrice == -1 ? SQL_NULL_DATA : 0;
		data.avgEstimatedSoldPriceNullIndicator = auctionInfo.avgEstimatedSoldPrice == -1 ? SQL_NULL_DATA : 0;
		data.itemNumber = auctionInfo.itemNumber;
		data.minPrice = auctionInfo.minPrice;
		data.maxPrice = auctionInfo.maxPrice;
		data.avgPrice = auctionInfo.avgPrice;
		data.minPriceNullIndicator = auctionInfo.minPrice == -1 ? SQL_NULL_DATA : 0;
		data.maxPriceNullIndicator = auctionInfo.maxPrice == -1 ? SQL_NULL_DATA : 0;
		data.avgPriceNullIndicator = auctionInfo.avgPrice == -1 ? SQL_NULL_DATA : 0;

		dataToWrite.push_back(data);
	}

	addResult(
	    item,
	    std::make_tuple(
	        std::move(std::get<0>(inputData)), std::move(std::get<1>(inputData)), std::move(std::get<2>(inputData))));

	dbQuery.executeDbQuery<DB_AuctionSummary>([this, item](auto, int status) { workDone(item, status); }, dataToWrite);

	// httpClientSession.sendData(fullUrl, std::get<3>(jsonData), [this, item]() { workDone(item, 0); });
	// workDone(item, 0);
}
