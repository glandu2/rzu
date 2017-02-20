#include "AuctionParser.h"
#include "Core/EventLoop.h"
#include "AuctionWriter.h"
#include <algorithm>
#include "Config/ConfigParamVal.h"
#include <time.h>

struct AuctionFile {
	size_t alreadyExistingAuctions;
	size_t addedAuctionsInFile;
	AUCTION_SIMPLE_FILE auctions;
	bool isFull;

	AuctionFile() {
		isFull = true;
		alreadyExistingAuctions = 0;
		addedAuctionsInFile = 0;
	}

	void adjustDetectedType(AuctionComplexDiffWriter* auctionWriter) {
		for(size_t i = 0; i < auctions.auctions.size(); i++) {
			const AUCTION_SIMPLE_INFO& auctionData = auctions.auctions[i];
			if(auctionData.diffType != D_Added && auctionData.diffType != D_Base)
				isFull = false;
			if(auctionData.diffType == D_Added && auctionWriter->hasAuction(AuctionUid(auctionData.uid)))
				alreadyExistingAuctions++;
			if(auctionData.diffType == D_Added || auctionData.diffType == D_Base)
				addedAuctionsInFile++;
		}

		if(alreadyExistingAuctions == 0 && addedAuctionsInFile < auctionWriter->getAuctionCount() / 2)
			isFull = false;
	}
};

AuctionParser::AuctionParser(IParser* aggregator,
                             cval<std::string>& auctionsPath,
                             cval<int>& changeWaitSeconds,
                             cval<std::string>& statesPath,
                             cval<std::string>& auctionStateFile,
                             cval<std::string>& aggregationStateFile)
    : auctionWriter(19),
      aggregator(aggregator),
      auctionsPath(auctionsPath),
      changeWaitSeconds(changeWaitSeconds),
      statesPath(statesPath),
      auctionStateFile(auctionStateFile),
      aggregationStateFile(aggregationStateFile),
      started(false)
{
	importState();
}

bool AuctionParser::start()
{
	if(!isStarted()) {
		started = true;
		log(LL_Info, "Starting watching auctions directory %s\n", auctionsPath.get().c_str());
		dirWatchTimer.start(this, &AuctionParser::onTimeout, 1000, 10000);
	}

	return true;
}

void AuctionParser::stop()
{
	log(LL_Info, "Stop watching auctions\n");
	started = false;
	dirWatchTimer.stop();
}

bool AuctionParser::isStarted()
{
	return started;
}

void AuctionParser::onTimeout()
{
	scandirReq.data = this;
	std::string path = auctionsPath.get();
	log(LL_Debug, "Checking %s for new files since file \"%s\"\n", path.c_str(), lastParsedFile.c_str());
	uv_fs_scandir(EventLoop::getLoop(), &scandirReq, path.c_str(), 0, &AuctionParser::onScandir);
}

void AuctionParser::onScandir(uv_fs_t* req)
{
	AuctionParser* thisInstance = (AuctionParser*) req->data;
	uv_dirent_t dent;

	if(req->result < 0) {
		thisInstance->log(LL_Error, "Failed to scan dir \"%s\", error: %s(%d)\n", req->path, uv_strerror(req->result), req->result);
		return;
	}

	std::string maxFile = thisInstance->lastParsedFile;
	int waitChangeSeconds = thisInstance->changeWaitSeconds.get();

	std::vector<std::string> orderedFiles;

	// Get all file names and order by name
	while (UV_EOF != uv_fs_scandir_next(req, &dent)) {
		if(dent.type != UV_DIRENT_DIR)
			orderedFiles.push_back(dent.name);
	}

	std::sort(orderedFiles.begin(), orderedFiles.end());

	auto it = orderedFiles.begin();
	auto itEnd = orderedFiles.end();
	for(; it != itEnd && !thisInstance->aggregator->isFull(); ++it) {
		const std::string& filename = *it;
		if(strcmp(thisInstance->lastParsedFile.c_str(), filename.c_str()) < 0) {
			uv_fs_t statReq;
			std::string fullFilename = thisInstance->auctionsPath.get() + "/" + filename;

			uv_fs_stat(EventLoop::getLoop(), &statReq, fullFilename.c_str(), nullptr);
			time_t timeLimit = time(nullptr) - waitChangeSeconds;
			if(statReq.statbuf.st_mtim.tv_sec > timeLimit ||
			        statReq.statbuf.st_ctim.tv_sec > timeLimit ||
			        statReq.statbuf.st_birthtim.tv_sec > timeLimit) {
				thisInstance->log(LL_Trace, "File %s too new before parsing (modified less than %d seconds: %ld)\n",
				                  filename.c_str(), waitChangeSeconds, statReq.statbuf.st_mtim.tv_sec);
				// don't parse file after this one if it is a new file (avoid parsing file in wrong order)
				break;
			}

			thisInstance->log(LL_Info, "Found new auction file %s\n", filename.c_str());
			if(!thisInstance->parseFile(fullFilename)) {
				thisInstance->log(LL_Error, "Parsing failed on %s, stopping parsing of other files\n", filename.c_str());
				break;
			}
			if(strcmp(maxFile.c_str(), filename.c_str()) < 0)
				maxFile = filename;
		}
	}

	if(thisInstance->aggregator->isFull()) {
		thisInstance->log(LL_Info, "Stopping parsing, aggregator is full (pending data to send to webserver)\n");
	}

	if(thisInstance->lastParsedFile != maxFile) {
		thisInstance->lastParsedFile = maxFile;
		thisInstance->log(LL_Info, "New last file now %s\n", maxFile.c_str());
		thisInstance->exportState();
	} else {
		thisInstance->log(LL_Debug, "No new auction since file %s\n", maxFile.c_str());
	}
}

bool AuctionParser::parseFile(std::string fullFilename)
{
	std::vector<uint8_t> data;
	int version;
	AuctionFileFormat fileFormat;
	AuctionFile auctionFile;

	log(LL_Debug, "Parsing file %s\n", fullFilename.c_str());

	if(!AuctionWriter::readAuctionDataFromFile(fullFilename, data)) {
		log(LL_Error, "Cant read file %s\n", fullFilename.c_str());
		return false;
	}

	if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
		log(LL_Error, "Invalid file, unrecognized header signature: %s\n", fullFilename.c_str());
		return false;
	}

	if(!AuctionWriter::deserialize(&auctionFile.auctions, data)) {
		log(LL_Error, "Can't deserialize file %s\n", fullFilename.c_str());
		return false;
	}

	auctionFile.adjustDetectedType(&auctionWriter);

	log(LL_Debug, "Processing file %s, detected type: %s, alreadyExistingAuctions: %d/%d, addedAuctionsInFile: %d/%d\n",
	    fullFilename.c_str(),
	    auctionFile.isFull ? "full" : "diff",
	    auctionFile.alreadyExistingAuctions,
	    auctionWriter.getAuctionCount(),
	    auctionFile.addedAuctionsInFile,
	    auctionWriter.getAuctionCount());

	for(size_t i = 0; i < auctionFile.auctions.header.categories.size(); i++) {
		const AUCTION_CATEGORY_INFO& category = auctionFile.auctions.header.categories[i];
		auctionWriter.beginCategory(i, category.beginTime);
		auctionWriter.endCategory(i, category.endTime);
	}

	auctionWriter.setDiffInputMode(!auctionFile.isFull);
	auctionWriter.beginProcess();

	for(size_t auction = 0; auction < auctionFile.auctions.auctions.size(); auction++) {
		const AUCTION_SIMPLE_INFO& auctionData = auctionFile.auctions.auctions[auction];
		auctionWriter.addAuctionInfo(&auctionData);
	}
	auctionWriter.endProcess();

	return aggregator->parseAuctions(&auctionWriter);
}

bool AuctionParser::importState()
{
	std::string auctionStateFile = this->auctionStateFile.get();
	std::string aggregationStateFile = this->aggregationStateFile.get();

	if(!auctionStateFile.empty()) {
		auctionStateFile = statesPath.get() + "/" + auctionStateFile;

		std::vector<uint8_t> data;

		if(AuctionWriter::readAuctionDataFromFile(auctionStateFile, data)) {
			AUCTION_FILE auctionFileData;
			if(!AuctionWriter::deserialize(&auctionFileData, data)) {
				log(LL_Error, "Can't deserialize state file %s\n", auctionStateFile.c_str());
			} else {
				log(LL_Info, "Loading auction state file %s with %d auctions\n", auctionStateFile.c_str(), auctionFileData.auctions.size());
				auctionWriter.importDump(&auctionFileData);
			}
		} else {
			log(LL_Error, "Cant read state file %s\n", auctionStateFile.c_str());
		}
	}

	if(!aggregationStateFile.empty()) {
		aggregationStateFile = statesPath.get() + "/" + aggregationStateFile;
		aggregator->importState(aggregationStateFile, lastParsedFile);
		log(LL_Info, "Last parsed file: %s\n", lastParsedFile.c_str());
	}

	return true;
}

void AuctionParser::exportState()
{
	std::string auctionStateFile = this->auctionStateFile.get();
	std::string aggregationStateFile = this->aggregationStateFile.get();

	if(!auctionStateFile.empty()) {
		auctionStateFile = statesPath.get() + "/" + auctionStateFile;
		log(LL_Info, "Writting auction state file %s\n", auctionStateFile.c_str());
		std::vector<uint8_t> data;
		auctionWriter.dumpAuctions(data, true, true);
		AuctionWriter::writeAuctionDataToFile(auctionStateFile, data);
	}

	if(!aggregationStateFile.empty()) {
		aggregationStateFile = statesPath.get() + "/" + aggregationStateFile;
		aggregator->exportState(aggregationStateFile, lastParsedFile);
	}
}
