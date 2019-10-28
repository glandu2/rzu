#include "P2ParseAuction.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include <errno.h>

#include "Core/Utils.h"

P2ParseAuction::P2ParseAuction()
    : PipelineStep<std::unique_ptr<AuctionFile>, std::unique_ptr<AuctionDumpToAggregate>>(100, 1, 10),
      work(this, &P2ParseAuction::processWork, &P2ParseAuction::afterWork),
      auctionWriter(19),
      currentDate(-1),
      previousWasNewDay(false),
      previousTime(0) {}

void P2ParseAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

void P2ParseAuction::importState(const AUCTION_FILE* auctionData) {
	auctionWriter.importDump(auctionData);
}

int P2ParseAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::unique_ptr<AuctionFile>& auctionFile : sources) {
		time_t dumpBeginTime;

		item->setName(auctionFile->filename);
		log(LL_Debug, "Parsing file %s\n", auctionFile->filename.c_str());

		if(currentDate == -1)
			doDump(item,
			       std::string(),
			       DAT_InitialDump,
			       0,
			       auctionWriter.getCategoryTimeManager().getEstimatedPreviousCategoryBeginTime(0));

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

		dumpBeginTime = auctionWriter.getCategoryTimeManager().getEstimatedPreviousCategoryBeginTime(0);

		if(dumpBeginTime == 0) {
			log(LL_Warning, "Begin time of first category is 0 after parsing file %s\n", auctionFile->filename.c_str());
			dumpBeginTime = currentDate;
		}

		// full dump if its a new day
		if(previousWasNewDay)
			doDump(item, auctionFile->filename, DAT_NewDay, previousTime, currentDate);
		else
			doDump(item, auctionFile->filename, DAT_Diff, currentDate, currentDate);

		previousTime = currentDate;
		previousWasNewDay = isNewDay(auctionFile->filename, dumpBeginTime);
	}

	return 0;
}

bool P2ParseAuction::isNewDay(const std::string& filename, time_t dumpBeginTime) {
	bool result = false;
	int dateCompareResult = compareWithCurrentDate(dumpBeginTime);
	if(dateCompareResult > 0) {
		if(currentDate != -1)
			result = true;  // New day only if not the first day
		currentDate = dumpBeginTime;
	} else if(dateCompareResult < 0) {
		log(LL_Warning,
		    "New date is smaller than current date: %d < %d at file %s\n",
		    (int) dumpBeginTime,
		    (int) currentDate,
		    filename.c_str());
	} else {
		log(LL_Debug, "Date from %d to %d not changed\n", (int) currentDate, (int) dumpBeginTime);
	}

	return result;
}

int P2ParseAuction::doDump(std::shared_ptr<WorkItem> item,
                           const std::string& filename,
                           enum DumpToAggregateType dumpType,
                           time_t previousTimestamp,
                           time_t timestamp) {
	std::unique_ptr<AuctionDumpToAggregate> auctionFileDump = std::make_unique<AuctionDumpToAggregate>();

	auctionFileDump->dumpType = dumpType;
	auctionFileDump->filename = filename;
	auctionFileDump->previousTimestamp = previousTimestamp;
	auctionFileDump->timestamp = timestamp;
	auctionFileDump->auctionFile = auctionWriter.exportDump(dumpType != DAT_Diff, true);

	if(auctionFileDump->auctionFile.auctions.empty()) {
		log(LL_Debug, "No new auction to parse\n");
		return 0;
	}

	addResult(item, std::move(auctionFileDump));

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
