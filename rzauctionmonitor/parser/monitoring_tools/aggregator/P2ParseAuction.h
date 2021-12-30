#pragma once

#include "AuctionParser.h"
#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "PacketIterator.h"
#include "PipelineState.h"

enum DumpToAggregateType { DAT_Diff, DAT_NewDay, DAT_InitialDump };

struct AuctionDumpToAggregate {
	time_t timestamp;  // timestamp of beginning of current dump
	std::vector<AUCTION_FILE> auctionFile;
};

class AuctionPipeline;

class P2ParseAuction
    : public PipelineStep<std::pair<PipelineState, AUCTION_SIMPLE_FILE>, std::pair<PipelineState, AUCTION_FILE>> {
	DECLARE_CLASSNAME(P2ParseAuction, 0)
public:
	P2ParseAuction();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;
	virtual void doCancelWork(std::shared_ptr<WorkItem> item) override { work.cancel(); }

	void importState(const AUCTION_FILE* auctionData);

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

	bool isNewDay(const std::string& filename, time_t dumpBeginTime);
	int compareWithCurrentDate(time_t other);

private:
	BackgroundWork<P2ParseAuction, std::shared_ptr<WorkItem>> work;

	AuctionComplexDiffWriter auctionWriter;
	time_t currentDate;
};
