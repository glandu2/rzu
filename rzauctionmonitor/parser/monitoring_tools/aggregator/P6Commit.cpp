#include "P6Commit.h"
#include "AuctionComplexDiffWriter.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Config/ConfigParamVal.h"
#include "Core/Utils.h"
#include <errno.h>

P6Commit::P6Commit(AuctionPipeline* auctionPipeline)
    : PipelineStep<PipelineAggregatedState, void>(1),
      auctionPipeline(auctionPipeline),
      work(this, &P6Commit::processWork, &P6Commit::afterWork) {}

void P6Commit::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P6Commit::processWork(std::shared_ptr<WorkItem> workItem) {
	PipelineAggregatedState sourceData = std::move(workItem->getSource());
	struct tm currentDay;

	workItem->setName(std::to_string(sourceData.base.timestamp));

	Utils::getGmTime(sourceData.base.timestamp, &currentDay);

	log(LL_Debug, "Saving state for date %02d/%02d/%04d\n", currentDay.tm_mday, currentDay.tm_mon, currentDay.tm_year);

	if(!sourceData.dumps.empty())
		return auctionPipeline->exportState(sourceData.base.lastFilenameParsed, sourceData.dumps[0]);
	else
		return -ENOENT;
}

void P6Commit::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
