#include "P3ParseAuction.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include <errno.h>

#include "Core/Utils.h"

P3ParseAuction::P3ParseAuction()
    : PipelineStep<std::pair<PipelineState, AUCTION_SIMPLE_FILE>, std::pair<PipelineState, AUCTION_FILE>>(100, 1, 10),
      work(this, &P3ParseAuction::processWork, &P3ParseAuction::afterWork),
      auctionWriter(19),
      timeSinceLastCommit(0) {}

void P3ParseAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

void P3ParseAuction::importState(const AUCTION_FILE* auctionData) {
	auctionWriter.importDump(auctionData);
}

int P3ParseAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<PipelineState, AUCTION_SIMPLE_FILE>& input : sources) {
		item->setName(input.first.lastFilenameParsed);
		log(LL_Debug, "Parsing file %s\n", input.first.lastFilenameParsed.c_str());
		bool isFull;

		isFull = AuctionFile::isFileFullType(input.second, &auctionWriter);

		log(LL_Info,
		    "Processing file %s, detected type: %s\n",
		    input.first.lastFilenameParsed.c_str(),
		    isFull ? "full" : "diff");

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

		AUCTION_FILE auctionDump = auctionWriter.exportDump(false, true);

		// Save state every 10s
		if(time(nullptr) > timeSinceLastCommit + 10) {
			log(LL_Info, "Doing full dump for state save\n");
			timeSinceLastCommit = time(nullptr);
			input.first.fullDump = auctionWriter.exportDump(true, true);
		}

		if(auctionDump.auctions.empty()) {
			log(LL_Debug, "No new auction to parse\n");
		} else {
			addResult(item, std::make_pair(std::move(input.first), std::move(auctionDump)));
		}
	}

	return 0;
}

void P3ParseAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
