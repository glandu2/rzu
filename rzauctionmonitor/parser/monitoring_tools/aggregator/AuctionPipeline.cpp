#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Core/EventLoop.h"
#include "NetSession/ServersManager.h"
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
                                 cval<std::string>& statesPath,
                                 cval<std::string>& auctionStateFile)
    : auctionsPath(auctionsPath),
      changeWaitSeconds(changeWaitSeconds),
      statesPath(statesPath),
      auctionStateFile(auctionStateFile),
      started(false),
      scanAuctionStep(changeWaitSeconds),
      sendToWebServerStep(),
      commitStep(this) {
	this->plug(&readAuctionStep)
	    ->plug(&deserializeAuctionStep)
	    ->plug(&parseAuctionStep)
	    ->plug(&aggregateStatsStep)
	    ->plug(&computeStatsStep)
	    ->plug(&sendHistoryToSqlStep)
	    ->plug(&sendToWebServerStep)
	    ->plug(&commitStep);
	uv_fs_event_init(EventLoop::getLoop(), &fsEvent);
	fsEvent.data = this;
}

bool AuctionPipeline::start() {
	if(!isStarted()) {
		started = true;
		importState();
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

void AuctionPipeline::importState() {
	std::string aggregationStateFile = this->auctionStateFile.get();

	if(aggregationStateFile.empty()) {
		return;
	}

	aggregationStateFile = statesPath.get() + "/" + aggregationStateFile;

	std::vector<AuctionWriter::file_data_byte> data;

	if(AuctionWriter::readAuctionDataFromFile(aggregationStateFile, data)) {
		PIPELINE_STATE aggregationState;

		if(data.size() < 2) {
			log(LL_Info, "Auction state file %s is too small\n", aggregationStateFile.c_str());
			return;
		}

		uint16_t version = *reinterpret_cast<const uint16_t*>(data.data());
		MessageBuffer buffer(data.data(), data.size(), version);
		aggregationState.deserialize(&buffer);
		if(buffer.checkFinalSize() == false) {
			log(LL_Error, "Can't deserialize state file %s\n", aggregationStateFile.c_str());
			log(LL_Error,
			    "Wrong buffer size, size: %u, field: %s\n",
			    buffer.getSize(),
			    buffer.getFieldInOverflow().c_str());
			return;
		}

		log(LL_Info,
		    "Loading auction state file %s with %d auctions\n",
		    aggregationStateFile.c_str(),
		    (int) aggregationState.auctionData.auctions.size());

		lastQueuedFile = aggregationState.lastParsedFile;
		parseAuctionStep.importState(&aggregationState.auctionData);
	} else {
		log(LL_Error, "Cant read state file %s\n", aggregationStateFile.c_str());
	}
}

int AuctionPipeline::exportState(std::string& fullFilename, AUCTION_FILE& auctionFile) {
	// Executed in thread
	PIPELINE_STATE aggregationState;

	std::string aggregationStateFile = this->auctionStateFile.get();

	if(aggregationStateFile.empty()) {
		return 0;
	}

	log(LL_Info, "Writing auction state file %s\n", aggregationStateFile.c_str());

	aggregationStateFile = statesPath.get() + "/" + aggregationStateFile;

	std::string filename;
	size_t lastSlash = fullFilename.find_last_of('/');
	if(lastSlash != std::string::npos) {
		filename = fullFilename.substr(lastSlash);
	} else {
		filename = fullFilename;
	}

	aggregationState.file_version = PIPELINE_STATE_VERSION;
	aggregationState.lastParsedFile = std::move(filename);
	aggregationState.auctionData = std::move(auctionFile);

	std::vector<uint8_t> data;
	data.resize(aggregationState.getSize(aggregationState.file_version));
	MessageBuffer buffer(data.data(), data.size(), aggregationState.file_version);
	aggregationState.serialize(&buffer);
	if(buffer.checkFinalSize() == false) {
		log(LL_Error,
		    "Wrong buffer size, size: %u, field: %s\n",
		    buffer.getSize(),
		    buffer.getFieldInOverflow().c_str());

		return ERANGE;
	}
	AuctionWriter::writeAuctionDataToFile(aggregationStateFile, data);

	return 0;
}

void AuctionPipeline::onTimeout() {
	//	if(readAuctionStep.isFull()) {
	//		log(LL_Debug, "Pipeline input is still full, not checking new files this time\n");
	//	} else {
	scandirReq.data = this;
	std::string path = auctionsPath.get();
	log(LL_Debug, "Checking %s for new files since file \"%s\"\n", path.c_str(), lastQueuedFile.c_str());

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
	while(UV_EOF != uv_fs_scandir_next(req, &dent)) {
		if(dent.type != UV_DIRENT_DIR && strcmp(thisInstance->lastQueuedFile.c_str(), dent.name) < 0)
			orderedFiles.push_back(dent.name);
		if(orderedFiles.size() > 10000)
			break;
	}

	std::sort(orderedFiles.begin(), orderedFiles.end(), [](const std::string& a, const std::string& b) {
		return strcmp(a.c_str(), b.c_str()) < 0;
	});

	auto it = orderedFiles.begin();
	auto itEnd = orderedFiles.end();

	for(; it != itEnd; ++it) {
		const std::string& filename = *it;

		std::string fullFilename = thisInstance->auctionsPath.get() + "/" + filename;

		thisInstance->lastQueuedFile = filename;
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
