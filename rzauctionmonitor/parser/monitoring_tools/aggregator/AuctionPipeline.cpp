#include "AuctionPipeline.h"
#include "AuctionWriter.h"
#include "Core/EventLoop.h"
#include <algorithm>
#include <errno.h>
#include <time.h>
#include <uv.h>

#define PIPELINE_STATE_VERSION 1

// clang-format off
#define PIPELINE_STATE_DEF(_) \
	_(simple)  (uint16_t, file_version) \
	_(simple)  (uint64_t, currentDate) \
	_(count)   (uint8_t, lastParsedFile) \
	_(dynstring)(lastParsedFile, false) \
	_(simple)  (AUCTION_FILE, auctionData)
CREATE_STRUCT(PIPELINE_STATE);
// clang-format on

AuctionPipeline::AuctionPipeline(cval<std::string>& auctionsPath,
                                 cval<int>& changeWaitSeconds,
                                 cval<std::string>& statesPath,
                                 cval<std::string>& auctionStateFile,
                                 cval<std::string>& ip,
                                 cval<int>& port,
                                 cval<std::string>& url,
                                 cval<std::string>& pwd)
    : auctionsPath(auctionsPath),
      changeWaitSeconds(changeWaitSeconds),
      statesPath(statesPath),
      auctionStateFile(auctionStateFile),
      started(false),
      sendToWebServerStep(ip, port, url, pwd),
      commitStep(this) {
	readAuctionStep.plug(&parseAuctionStep)
	    ->plug(&aggregateStatsStep)
	    ->plug(&computeStatsStep)
	    ->plug(&sendToWebServerStep)
	    ->plug(&commitStep);
}

bool AuctionPipeline::start() {
	if(!isStarted()) {
		started = true;
		importState();
		log(LL_Info, "Starting watching auctions directory %s\n", auctionsPath.get().c_str());
		dirWatchTimer.start(this, &AuctionPipeline::onTimeout, 1000, 10000);
	}

	return true;
}

void AuctionPipeline::stop() {
	log(LL_Info, "Stop watching auctions\n");
	started = false;
	dirWatchTimer.stop();
	readAuctionStep.cancel();
}

bool AuctionPipeline::isStarted() {
	return started;
}

void AuctionPipeline::commit(std::unique_ptr<IPipelineOutput<std::pair<std::string, std::string> > > item, int status) {
	log(LL_Info, "File %s processed with status %d\n", item->getData().first.c_str(), status);
	if(status != 0)
		stop();
}

void AuctionPipeline::importState() {
	std::string aggregationStateFile = this->auctionStateFile.get();

	if(aggregationStateFile.empty()) {
		return;
	}

	aggregationStateFile = statesPath.get() + "/" + aggregationStateFile;

	std::vector<uint8_t> data;

	if(AuctionWriter::readAuctionDataFromFile(aggregationStateFile, data)) {
		PIPELINE_STATE aggregationState;
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

int AuctionPipeline::exportState(const std::string& fullFilename, time_t timestamp, const AUCTION_FILE& auctionFile) {
	// Executed in thread
	PIPELINE_STATE aggregationState;

	std::string aggregationStateFile = this->auctionStateFile.get();

	if(aggregationStateFile.empty()) {
		return 0;
	}

	aggregationStateFile = statesPath.get() + "/" + aggregationStateFile;

	log(LL_Info, "Writting auction state file %s\n", aggregationStateFile.c_str());

	std::string filename;
	size_t lastSlash = fullFilename.find_last_of('/');
	if(lastSlash != std::string::npos) {
		filename = fullFilename.substr(lastSlash);
	} else {
		filename = fullFilename;
	}

	aggregationState.file_version = PIPELINE_STATE_VERSION;
	aggregationState.lastParsedFile = fullFilename;
	aggregationState.currentDate = timestamp;
	aggregationState.auctionData = auctionFile;

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
	scandirReq.data = this;
	std::string path = auctionsPath.get();
	log(LL_Debug, "Checking %s for new files since file \"%s\"\n", path.c_str(), lastQueuedFile.c_str());
	uv_fs_scandir(EventLoop::getLoop(), &scandirReq, path.c_str(), 0, &AuctionPipeline::onScandir);
}

void AuctionPipeline::onScandir(uv_fs_t* req) {
	AuctionPipeline* thisInstance = (AuctionPipeline*) req->data;
	uv_dirent_t dent;

	if(req->result < 0) {
		thisInstance->log(LL_Error,
		                  "Failed to scan dir \"%s\", error: %s(%d)\n",
		                  req->path,
		                  uv_strerror(req->result),
		                  (int) req->result);
		return;
	}

	int waitChangeSeconds = thisInstance->changeWaitSeconds.get();

	std::vector<std::string> orderedFiles;

	// Get all file names and order by name
	while(UV_EOF != uv_fs_scandir_next(req, &dent)) {
		if(dent.type != UV_DIRENT_DIR)
			orderedFiles.push_back(dent.name);
	}

	std::sort(orderedFiles.begin(), orderedFiles.end(), [](const std::string& a, const std::string& b) {
		return strcmp(a.c_str(), b.c_str()) < 0;
	});

	auto it = orderedFiles.begin();
	auto itEnd = orderedFiles.end();

	for(; it != itEnd; ++it) {
		const std::string& filename = *it;
		if(strcmp(thisInstance->lastQueuedFile.c_str(), filename.c_str()) < 0) {
			uv_fs_t statReq;
			std::string fullFilename = thisInstance->auctionsPath.get() + "/" + filename;

			uv_fs_stat(EventLoop::getLoop(), &statReq, fullFilename.c_str(), nullptr);
			uv_fs_req_cleanup(&statReq);

			time_t timeLimit = time(nullptr) - waitChangeSeconds;

			if(statReq.statbuf.st_mtim.tv_sec > timeLimit || statReq.statbuf.st_ctim.tv_sec > timeLimit ||
			   statReq.statbuf.st_birthtim.tv_sec > timeLimit) {
				thisInstance->log(LL_Trace,
				                  "File %s too new before parsing (modified less than %d seconds: %ld)\n",
				                  filename.c_str(),
				                  waitChangeSeconds,
				                  statReq.statbuf.st_mtim.tv_sec);
				// don't parse file after this one if it is a new file (avoid parsing file in wrong order)
				break;
			}

			thisInstance->log(LL_Info, "Found new auction file %s\n", filename.c_str());
			thisInstance->lastQueuedFile = filename;
			thisInstance->readAuctionStep.queue(std::make_pair(filename, fullFilename), thisInstance);
		}
	}
}
