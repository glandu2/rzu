#ifndef P0SCANAUCTIONS_H
#define P0SCANAUCTIONS_H

#include "AuctionParser.h"
#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"

class AuctionPipeline;

class P0ScanAuctions : public PipelineStep<std::pair<std::string, std::string>, std::pair<std::string, std::string>> {
	DECLARE_CLASSNAME(P0ScanAuctions, 0)
public:
	P0ScanAuctions(cval<int>& changeWaitSeconds);
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	static void onFsStatStatic(uv_fs_t* req);
	void onFsStat(uv_fs_t* req);

private:
	uv_fs_t statReq;
	cval<int>& changeWaitSeconds;

	std::shared_ptr<PipelineStep::WorkItem> currentItem;
};

#endif
