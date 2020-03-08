#ifndef P12DESERIALIZEAUCTION_H
#define P12DESERIALIZEAUCTION_H

#include "AuctionParser.h"
#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "PipelineState.h"

class P12DeserializeAuction : public PipelineStep<std::pair<PipelineState, std::vector<uint8_t>>,
                                                  std::pair<PipelineState, AUCTION_SIMPLE_FILE>> {
	DECLARE_CLASSNAME(P12DeserializeAuction, 0)
public:
	P12DeserializeAuction();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

private:
	BackgroundWork<P12DeserializeAuction, std::shared_ptr<WorkItem>> work;
};

#endif
