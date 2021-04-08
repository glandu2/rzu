#pragma once

#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "P3AggregateStats.h"
#include <stdint.h>

class AuctionPipeline;

class P8Commit : public PipelineStep<PipelineAggregatedState, void> {
	DECLARE_CLASSNAME(P8Commit, 0)
public:
	P8Commit(AuctionPipeline* auctionPipeline);
	virtual void doWork(std::shared_ptr<WorkItem> item) override;
	virtual void doCancelWork(std::shared_ptr<WorkItem> item) override { work.cancel(); }

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

private:
	AuctionPipeline* auctionPipeline;
	BackgroundWork<P8Commit, std::shared_ptr<WorkItem>> work;
};
