#pragma once

#include "AuctionParser.h"
#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "PipelineState.h"

class AuctionPipeline;

class P3ParseAuction
    : public PipelineStep<std::pair<PipelineState, AUCTION_SIMPLE_FILE>, std::pair<PipelineState, AUCTION_FILE>> {
	DECLARE_CLASSNAME(P3ParseAuction, 0)
public:
	P3ParseAuction();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

	void importState(const AUCTION_FILE* auctionData);

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

private:
	BackgroundWork<P3ParseAuction, std::shared_ptr<WorkItem>> work;

	AuctionComplexDiffWriter auctionWriter;
	size_t fileNumberProcessedSinceLastCommit;
};

