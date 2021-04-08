#include "AuctionComplexDiffWriter.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Config/ConfigParamVal.h"
#include "Core/Utils.h"
#include "P8Commit.h"
#include <errno.h>

P8Commit::P8Commit(AuctionPipeline* auctionPipeline)
    : PipelineStep<PipelineAggregatedState, void>(1),
      auctionPipeline(auctionPipeline),
      work(this, &P8Commit::processWork, &P8Commit::afterWork) {}

void P8Commit::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P8Commit::processWork(std::shared_ptr<WorkItem> workItem) {
	PipelineAggregatedState sourceData = std::move(workItem->getSource());
	struct tm currentDay;

	workItem->setName(std::to_string(sourceData.base.timestamp));

	Utils::getGmTime(sourceData.base.timestamp, &currentDay);

	log(LL_Debug, "Saving state for date %02d/%02d/%04d\n", currentDay.tm_mday, currentDay.tm_mon, currentDay.tm_year);

	if(!sourceData.dumps.empty())
		return auctionPipeline->exportState(sourceData.base.associatedFilename, sourceData.dumps[0]);
	else
		return -ENOENT;
}

void P8Commit::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
