#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Core/EventLoop.h"
#include <NetSession/ServersManager.h>
#include <algorithm>
#include <errno.h>
#include <time.h>
#include <uv.h>

#define PIPELINE_STATE_VERSION AUCTION_LATEST

// clang-format off
#define PIPELINE_STATE_DEF(_) \
	_(simple)  (uint16_t, file_version) \
	_(count)   (uint8_t, lastParsedFile) \
	_(dynstring)(lastParsedFile, false) \
	_(simple)  (AUCTION_FILE, auctionData)
CREATE_STRUCT(PIPELINE_STATE);
#undef PIPELINE_STATE_DEF
// clang-format on

AuctionPipeline::AuctionPipeline(cval<std::string>& auctionsPath,
                                 cval<int>& changeWaitSeconds,
                                 cval<std::string>& lastQueuedFile)
    : auctionsPath(auctionsPath), changeWaitSeconds(changeWaitSeconds), lastQueuedFile(lastQueuedFile), started(false) {
	this->plug(&readAuctionStep)->plug(&deserializeAuctionStep);
	uv_fs_event_init(EventLoop::getLoop(), &fsEvent);
	fsEvent.data = this;
}

bool AuctionPipeline::start() {
	if(!isStarted()) {
		started = true;
		log(LL_Info, "Starting watching auctions directory %s\n", auctionsPath.get().c_str());
		// dirWatchTimer.start(this, &AuctionPipeline::onTimeout, 1000, 0);
		uv_fs_event_start(&fsEvent, &AuctionPipeline::onFsEvent, auctionsPath.get().c_str(), 0);
		onTimeout();
	}

	return true;
}

void AuctionPipeline::stop() {
	log(LL_Info, "Stop watching auctions\n");
	started = false;
	uv_fs_event_stop(&fsEvent);
	dirWatchTimer.stop();
	readAuctionStep.cancel();
}

bool AuctionPipeline::isStarted() {
	return started;
}

void AuctionPipeline::notifyError(int status) {
	if(started)
		ServersManager::getInstance()->forceStop();
}

void AuctionPipeline::notifyOutputAvailable() {}

void AuctionPipeline::onTimeout() {
	//	if(readAuctionStep.isFull()) {
	//		log(LL_Debug, "Pipeline input is still full, not checking new files this time\n");
	//	} else {
	scandirReq.data = this;
	std::string path = auctionsPath.get();
	log(LL_Debug, "Checking %s for new files since file \"%s\"\n", path.c_str(), lastQueuedFile.get().c_str());

	uv_fs_scandir(EventLoop::getLoop(), &scandirReq, path.c_str(), 0, &AuctionPipeline::onScandir);
	//	}
}

void AuctionPipeline::onScandir(uv_fs_t* req) {
	AuctionPipeline* thisInstance = (AuctionPipeline*) req->data;
	uv_dirent_t dent;

	if(req->result < 0) {
		thisInstance->log(LL_Error,
		                  "Failed to scan dir \"%s\", error: %s(%d)\n",
		                  req->path,
		                  uv_strerror(static_cast<int>(req->result)),
		                  (int) req->result);
		return;
	}

	std::vector<std::string> orderedFiles;

	// Get all file names after lastQueuedFile and order by name
	std::string lastQueuedFile = thisInstance->lastQueuedFile.get();
	while(UV_EOF != uv_fs_scandir_next(req, &dent)) {
		if(dent.type != UV_DIRENT_DIR && strcmp(lastQueuedFile.c_str(), dent.name) <= 0)
			orderedFiles.push_back(dent.name);
		//#warning "TODO remove"
		//		if(orderedFiles.size() > 1000)
		//			break;
	}

	std::sort(orderedFiles.begin(), orderedFiles.end(), [](const std::string& a, const std::string& b) {
		return strcmp(a.c_str(), b.c_str()) < 0;
	});

	auto it = orderedFiles.begin();
	auto itEnd = orderedFiles.end();

	for(; it != itEnd; ++it) {
		std::string& filename = *it;

		std::string fullFilename = thisInstance->auctionsPath.get() + "/" + filename;

		thisInstance->readAuctionStep.queue(std::make_pair(std::move(filename), std::move(fullFilename)));
	}
}

void AuctionPipeline::onFsEvent(uv_fs_event_t* handle, const char* filename, int events, int status) {
	AuctionPipeline* thisInstance = (AuctionPipeline*) handle->data;

	int waitChangeSeconds = thisInstance->changeWaitSeconds.get();
	thisInstance->log(
	    LL_Info, "Change in auctions input directory detected, triggering scandir in %ds\n", waitChangeSeconds);
	thisInstance->dirWatchTimer.start(thisInstance, &AuctionPipeline::onTimeout, waitChangeSeconds * 1000, 0);
}
