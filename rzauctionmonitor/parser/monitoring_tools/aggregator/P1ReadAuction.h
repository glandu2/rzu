#ifndef P1READAUCTION_H
#define P1READAUCTION_H

#include "AuctionParser.h"
#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"

class P1ReadAuction
    : public PipelineStep<std::pair<std::string, std::string>, std::unique_ptr<AuctionFile>, std::string>,
      public Object {
	DECLARE_CLASSNAME(P1ReadAuction, 0)
public:
	P1ReadAuction();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

private:
	BackgroundWork<P1ReadAuction, std::shared_ptr<WorkItem>> work;
};

#endif
