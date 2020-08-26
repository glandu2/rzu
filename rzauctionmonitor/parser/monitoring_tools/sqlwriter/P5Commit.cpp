#include "P5Commit.h"
#include "AuctionComplexDiffWriter.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Config/ConfigParamVal.h"
#include "Core/Utils.h"

P5Commit::P5Commit(AuctionPipeline* auctionPipeline)
    : PipelineStep<PipelineState, void>(10, 1),
      auctionPipeline(auctionPipeline),
      work(this, &P5Commit::processWork, &P5Commit::afterWork) {}

void P5Commit::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P5Commit::processWork(std::shared_ptr<WorkItem> workItem) {
	auto sources = std::move(workItem->getSources());
	PipelineState sourceData = sources.back();

	if(sourceData.fullDump.has_value()) {
		workItem->setName(sourceData.lastFilenameParsed);

		log(LL_Info, "Saving state in file %s\n", sourceData.lastFilenameParsed.c_str());

		return auctionPipeline->exportState(sourceData.lastFilenameParsed, sourceData.fullDump.value());
	} else {
		return 0;
	}
}

void P5Commit::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
