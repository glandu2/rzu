#include "P2ParseAuction.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include <errno.h>

#include "Core/PrintfFormats.h"
#include "Core/Utils.h"

P2ParseAuction::P2ParseAuction()
    : PipelineStep<std::pair<PipelineState, AUCTION_SIMPLE_FILE>, std::pair<PipelineState, AUCTION_FILE>>(10, 1, 1),
      work(this, &P2ParseAuction::processWork, &P2ParseAuction::afterWork),
      auctionWriter(19),
      currentDate(-1) {}

void P2ParseAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

void P2ParseAuction::importState(const AUCTION_FILE* auctionData) {
	auctionWriter.importDump(auctionData);
	currentDate = auctionData->header.categories.front().beginTime;
}

int P2ParseAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<PipelineState, AUCTION_SIMPLE_FILE>& input : sources) {
		const std::string& filename = input.first.associatedFilename;
		time_t dumpBeginTime;
		bool isFull;

		item->setName(filename);
		log(LL_Debug, "Parsing file %s\n", filename.c_str());

		isFull = AuctionFile::isFileFullType(input.second, &auctionWriter);

		log(LL_Info, "Processing file %s, detected type: %s\n", filename.c_str(), isFull ? "full" : "diff");

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

		log(LL_Debug, "File %s dumpBeginTime: %" PRIu64 "\n", filename.c_str(), dumpBeginTime);

		if(dumpBeginTime == 0) {
			log(LL_Warning, "Begin time of first category is 0 after parsing file %s\n", filename.c_str());
			dumpBeginTime = currentDate;
		} else {
			currentDate = dumpBeginTime;
		}

		PipelineState pipelineState;
		pipelineState.timestamp = dumpBeginTime;
		pipelineState.associatedFilename = input.first.associatedFilename;
		addResult(item, std::make_pair(std::move(pipelineState), auctionWriter.exportDump(false, false)));
	}

	return 0;
}

void P2ParseAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
