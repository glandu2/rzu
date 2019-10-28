#include "P6Commit.h"
#include "AuctionComplexDiffWriter.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Config/ConfigParamVal.h"
#include "Core/Utils.h"

P6Commit::P6Commit(AuctionPipeline* auctionPipeline)
    : PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE>, void>(10),
      auctionPipeline(auctionPipeline),
      work(this, &P6Commit::processWork, &P6Commit::afterWork) {}

void P6Commit::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P6Commit::processWork(std::shared_ptr<WorkItem> workItem) {
	std::tuple<std::string, time_t, AUCTION_FILE> sourceData = std::move(workItem->getSource());
	struct tm currentDay;

	workItem->setName(std::to_string(std::get<1>(sourceData)));

	Utils::getGmTime(std::get<1>(sourceData), &currentDay);

	log(LL_Debug, "Saving state for date %02d/%02d/%04d\n", currentDay.tm_mday, currentDay.tm_mon, currentDay.tm_year);

	return auctionPipeline->exportState(std::get<0>(sourceData), std::get<2>(sourceData));
}

void P6Commit::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
