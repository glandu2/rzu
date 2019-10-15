#include "P2ParseAuction.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include <errno.h>

#include "Core/Utils.h"

P2ParseAuction::P2ParseAuction()
    : PipelineStep<std::unique_ptr<AuctionFile>, std::unique_ptr<AuctionDumpToAggregate>, char>(2),
      work(this, &P2ParseAuction::processWork, &P2ParseAuction::afterWork),
      auctionWriter(19),
      currentDate(-1) {}

void P2ParseAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

void P2ParseAuction::importState(const AUCTION_FILE* auctionData) {
	auctionWriter.importDump(auctionData);
}

int P2ParseAuction::processWork(std::shared_ptr<WorkItem> item) {
	AuctionFile* auctionFile = item->getSource().get();
	time_t dumpBeginTime = -1;

	log(LL_Debug, "Parsing file %s\n", auctionFile->filename.c_str());

	for(size_t i = 0; i < auctionFile->auctions.header.categories.size(); i++) {
		const AUCTION_CATEGORY_INFO& category = auctionFile->auctions.header.categories[i];
		if(dumpBeginTime == -1 || dumpBeginTime < category.endTime)
			dumpBeginTime = category.endTime;
	}

	if(dumpBeginTime == -1) {
		log(LL_Error, "No category timestamp\n");
		return EINVAL;
	}

	// full dump if its a new day
	int dateCompareResult = compareWithCurrentDate(dumpBeginTime);
	if(dateCompareResult > 0) {
		if(currentDate != -1)
			doDump(item, auctionFile->filename, true, currentDate, dumpBeginTime);
		currentDate = dumpBeginTime;
	} else if(dateCompareResult < 0) {
		log(LL_Warning, "New date is smaller than current date: %d < %d\n", (int) dumpBeginTime, (int) currentDate);
	} else {
		log(LL_Debug, "Date from %d to %d not changed\n", (int) currentDate, (int) dumpBeginTime);
	}

	auctionFile->adjustDetectedType(&auctionWriter);

	log(LL_Debug,
	    "Processing file %s, detected type: %s, alreadyExistingAuctions: %d/%d, addedAuctionsInFile: %d/%d\n",
	    auctionFile->filename.c_str(),
	    auctionFile->isFull ? "full" : "diff",
	    (int) auctionFile->alreadyExistingAuctions,
	    (int) auctionWriter.getAuctionCount(),
	    (int) auctionFile->addedAuctionsInFile,
	    (int) auctionWriter.getAuctionCount());

	for(size_t i = 0; i < auctionFile->auctions.header.categories.size(); i++) {
		const AUCTION_CATEGORY_INFO& category = auctionFile->auctions.header.categories[i];
		auctionWriter.beginCategory(i, category.beginTime);
		auctionWriter.endCategory(i, category.endTime);
	}

	auctionWriter.setDiffInputMode(!auctionFile->isFull);
	auctionWriter.beginProcess();

	for(size_t auction = 0; auction < auctionFile->auctions.auctions.size(); auction++) {
		const AUCTION_SIMPLE_INFO& auctionData = auctionFile->auctions.auctions[auction];
		auctionWriter.addAuctionInfo(&auctionData);
	}

	auctionWriter.endProcess();

	doDump(item, auctionFile->filename, false, currentDate, currentDate);

	return 0;
}

int P2ParseAuction::doDump(std::shared_ptr<WorkItem> item,
                           const std::string& filename,
                           bool doFullDump,
                           time_t previousTimestamp,
                           time_t timestamp) {
	std::unique_ptr<AuctionDumpToAggregate> auctionFileDump = std::make_unique<AuctionDumpToAggregate>();

	auctionFileDump->isNewDay = doFullDump;
	auctionFileDump->filename = filename;
	auctionFileDump->previousTimestamp = previousTimestamp;
	auctionFileDump->timestamp = timestamp;
	auctionFileDump->auctionFile = auctionWriter.exportDump(doFullDump, true);

	if(auctionFileDump->auctionFile.auctions.empty()) {
		log(LL_Debug, "No new auction to parse\n");
		return 0;
	}

	addResult(item, std::move(auctionFileDump), 0);

	return 0;
}

int P2ParseAuction::compareWithCurrentDate(time_t other) {
	struct tm currentDateTm;
	struct tm otherDateTm;

	Utils::getGmTime(currentDate, &currentDateTm);
	Utils::getGmTime(other, &otherDateTm);

	if(currentDate == -1) {
		log(LL_Debug, "New date at %04d-%02d-%02d\n", otherDateTm.tm_year, otherDateTm.tm_mon, otherDateTm.tm_mday);
		return 1;
	}

	log(LL_Debug,
	    "Date changed from %04d-%02d-%02d to %04d-%02d-%02d\n",
	    currentDateTm.tm_year,
	    currentDateTm.tm_mon,
	    currentDateTm.tm_mday,
	    otherDateTm.tm_year,
	    otherDateTm.tm_mon,
	    otherDateTm.tm_mday);

	if(otherDateTm.tm_year != currentDateTm.tm_year)
		return otherDateTm.tm_year - currentDateTm.tm_year;

	if(otherDateTm.tm_mon != currentDateTm.tm_mon)
		return otherDateTm.tm_mon - currentDateTm.tm_mon;

	return otherDateTm.tm_mday - currentDateTm.tm_mday;
}

void P2ParseAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
