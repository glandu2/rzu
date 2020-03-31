#include "P0ScanAuctions.h"
#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Config/ConfigParamVal.h"
#include <errno.h>

P0ScanAuctions::P0ScanAuctions(cval<int>& changeWaitSeconds)
    : PipelineStep<std::pair<std::string, std::string>, std::pair<std::string, std::string>>(10000, 1),
      changeWaitSeconds(changeWaitSeconds) {
	statReq.data = this;
}

void P0ScanAuctions::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	this->currentItem = item;

	const std::pair<std::string, std::string>& filenames = item->getSource();
	// work.run(item);

	uv_fs_stat(EventLoop::getLoop(), &statReq, filenames.second.c_str(), &P0ScanAuctions::onFsStatStatic);
}

void P0ScanAuctions::onFsStatStatic(uv_fs_t* req) {
	P0ScanAuctions* thisInstance = (P0ScanAuctions*) req->data;
	thisInstance->onFsStat(req);
}

void P0ScanAuctions::onFsStat(uv_fs_t* req) {
	std::pair<std::string, std::string> filenames = std::move(currentItem->getSource());
	const std::string& filename = filenames.first;
	int waitChangeSeconds = changeWaitSeconds.get();

	time_t timeLimit = time(nullptr) - waitChangeSeconds;

	if(statReq.statbuf.st_mtim.tv_sec > timeLimit || statReq.statbuf.st_ctim.tv_sec > timeLimit ||
	   statReq.statbuf.st_birthtim.tv_sec > timeLimit) {
		log(LL_Trace,
		    "File %s too new before parsing (modified less than %d seconds: %ld)\n",
		    filename.c_str(),
		    waitChangeSeconds,
		    statReq.statbuf.st_mtim.tv_sec);
		// don't parse file after this one if it is a new file (avoid parsing file in wrong order)
		clear();
		workDone(currentItem, ERANGE);
	} else {
		addResult(currentItem, std::move(filenames));
		workDone(currentItem, 0);
	}

	uv_fs_req_cleanup(req);
}
