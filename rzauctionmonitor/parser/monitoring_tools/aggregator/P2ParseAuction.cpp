#include "P2ParseAuction.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include <errno.h>

#include "Core/Utils.h"

P2ParseAuction::P2ParseAuction()
    : PipelineStep<std::pair<PipelineState, AUCTION_SIMPLE_FILE>, std::pair<PipelineState, std::vector<AUCTION_FILE>>>(
          10, 1, 1),
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
	aggregatedDumps.clear();
	aggregatedDumps.emplace_back(*auctionData);
}

int P2ParseAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<PipelineState, AUCTION_SIMPLE_FILE>& input : sources) {
		const std::string& filename = input.first.lastFilenameParsed;
		time_t dumpBeginTime;
		bool isFull;

		item->setName(filename);
		log(LL_Debug, "Parsing file %s\n", filename.c_str());

		isFull = AuctionFile::isFileFullType(input.second, &auctionWriter);

		log(LL_Debug, "Processing file %s, detected type: %s\n", filename.c_str(), isFull ? "full" : "diff");

		for(size_t i = 0; i < input.second.header.categories.size(); i++) {
			const AUCTION_CATEGORY_INFO& category = input.second.header.categories[i];
			auctionWriter.beginCategory(i, category.beginTime);
			auctionWriter.endCategory(i, category.endTime);
		}

		auctionWriter.setDiffInputMode(!isFull);
		auctionWriter.beginProcess();

		for(size_t auction = 0; auction < input.second.auctions.size(); auction++) {
			const AUCTION_SIMPLE_INFO& auctionData = input.second.auctions[auction];
			auctionWriter.addAuctionInfo(&auctionData);
		}

		auctionWriter.endProcess();

		dumpBeginTime = auctionWriter.getCategoryTimeManager().getEstimatedPreviousCategoryBeginTime(0);

		if(dumpBeginTime == 0) {
			log(LL_Warning, "Begin time of first category is 0 after parsing file %s\n", filename.c_str());
			dumpBeginTime = currentDate;
		}

		AUCTION_FILE auctionFile;
		// full dump if its a new day
		if(previousWasNewDay) {
			input.first.timestamp = previousTime;
			addResult(item, std::make_pair(std::move(input.first), std::move(aggregatedDumps)));

			aggregatedDumps.clear();
		}

		if(aggregatedDumps.empty()) {
			aggregatedDumps.emplace_back(auctionWriter.exportDump(true, true));
		} else {
			aggregatedDumps.emplace_back(auctionWriter.exportDump(false, true));
		}

		previousTime = currentDate;
		previousWasNewDay = isNewDay(filename, dumpBeginTime);
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
