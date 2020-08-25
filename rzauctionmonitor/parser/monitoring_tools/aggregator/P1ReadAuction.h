#pragma once

#include "AuctionParser.h"
#include "AuctionWriter.h"
#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "PipelineState.h"

class P1ReadAuction : public PipelineStep<std::pair<std::string, std::string>,
                                          std::pair<PipelineState, std::vector<AuctionWriter::file_data_byte>>> {
	DECLARE_CLASSNAME(P1ReadAuction, 0)
public:
	P1ReadAuction();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;
	virtual void doCancelWork(std::shared_ptr<WorkItem> item) override { work.cancel(); }

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

private:
	BackgroundWork<P1ReadAuction, std::shared_ptr<WorkItem>> work;
};
