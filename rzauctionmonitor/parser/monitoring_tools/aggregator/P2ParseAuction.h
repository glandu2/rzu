#ifndef P2PARSEAUCTION_H
#define P2PARSEAUCTION_H

#include "AuctionParser.h"
#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"

enum DumpToAggregateType { DAT_Diff, DAT_NewDay, DAT_InitialDump };

struct AuctionDumpToAggregate {
	enum DumpToAggregateType dumpType;
	std::string filename;
	time_t previousTimestamp;  // timestamp of previous dump for stats computation when isNewDay == true
	time_t timestamp;          // timestamp of beginning of current dump
	AUCTION_FILE auctionFile;
};

class AuctionPipeline;

class P2ParseAuction : public PipelineStep<std::unique_ptr<AuctionFile>, std::unique_ptr<AuctionDumpToAggregate>> {
	DECLARE_CLASSNAME(P2ParseAuction, 0)
public:
	P2ParseAuction();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

	void importState(const AUCTION_FILE* auctionData);

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

	bool isNewDay(const std::string& filename, time_t dumpBeginTime);
	int doDump(std::shared_ptr<WorkItem> item,
	           const std::string& filename,
	           enum DumpToAggregateType dumpType,
	           time_t previousTimestamp,
	           time_t timestamp);
	int compareWithCurrentDate(time_t other);

private:
	BackgroundWork<P2ParseAuction, std::shared_ptr<WorkItem>> work;

	AuctionComplexDiffWriter auctionWriter;
	time_t currentDate;
	bool previousWasNewDay;
	time_t previousTime;
};

#endif
