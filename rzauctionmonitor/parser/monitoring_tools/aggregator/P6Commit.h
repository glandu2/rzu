#pragma once

#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "P3AggregateStats.h"
#include <stdint.h>

class AuctionPipeline;

class P6Commit : public PipelineStep<PipelineAggregatedState, void> {
	DECLARE_CLASSNAME(P6Commit, 0)
public:
	P6Commit(AuctionPipeline* auctionPipeline);
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

private:
	AuctionPipeline* auctionPipeline;
	BackgroundWork<P6Commit, std::shared_ptr<WorkItem>> work;
};

