#include "P1ReadAuction.h"
#include "AuctionWriter.h"

P1ReadAuction::P1ReadAuction()
    : PipelineStep<std::pair<std::string, std::string>, std::unique_ptr<AuctionFile>, std::string>(2),
      work(this, &P1ReadAuction::processWork, &P1ReadAuction::afterWork) {}

void P1ReadAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P1ReadAuction::processWork(std::shared_ptr<WorkItem> item) {
	const std::pair<std::string, std::string>& filenames = item->getSource();
	const std::string& filename = filenames.first;
	const std::string& fullFilename = filenames.second;
	std::vector<uint8_t> data;
	int version;
	AuctionFileFormat fileFormat;
	std::unique_ptr<AuctionFile> auctionFile{new AuctionFile};

	log(LL_Debug, "Reading file %s\n", filename.c_str());

	if(!AuctionWriter::readAuctionDataFromFile(fullFilename, data)) {
		log(LL_Error, "Cant read file %s\n", filename.c_str());
		return EIO;
	}

	if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
		log(LL_Error, "Invalid file, unrecognized header signature: %s\n", filename.c_str());
		return EBADMSG;
	}

	if(!AuctionWriter::deserialize(&auctionFile->auctions, data)) {
		log(LL_Error, "Can't deserialize file %s\n", filename.c_str());
		return EILSEQ;
	}

	auctionFile->filename = filename;

	addResult(item, std::move(auctionFile), fullFilename);

	return 0;
}

void P1ReadAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
