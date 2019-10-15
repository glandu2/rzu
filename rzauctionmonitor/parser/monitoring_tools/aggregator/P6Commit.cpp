#include "P6Commit.h"
#include "AuctionComplexDiffWriter.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Config/ConfigParamVal.h"

P6Commit::P6Commit(AuctionPipeline* auctionPipeline)
    : PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE>, char, char>(2),
      auctionPipeline(auctionPipeline),
      work(this, &P6Commit::processWork, &P6Commit::afterWork) {}

void P6Commit::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P6Commit::processWork(std::shared_ptr<WorkItem> workItem) {
	const std::tuple<std::string, time_t, AUCTION_FILE>& sourceData = workItem->getSource();

	log(LL_Debug, "Saving state for date %llu\n", (int64_t) std::get<1>(sourceData));

	return auctionPipeline->exportState(std::get<0>(sourceData), std::get<1>(sourceData), std::get<2>(sourceData));
}

void P6Commit::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
