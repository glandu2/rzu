#include "P12DeserializeAuction.h"
#include "AuctionWriter.h"

P12DeserializeAuction::P12DeserializeAuction()
    : PipelineStep<std::pair<std::string, std::vector<uint8_t>>, std::unique_ptr<AuctionFile>>(100, 1, 10),
      work(this, &P12DeserializeAuction::processWork, &P12DeserializeAuction::afterWork) {}

void P12DeserializeAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P12DeserializeAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<std::string, std::vector<uint8_t>>& input : sources) {
		std::string& filename = input.first;
		const std::vector<uint8_t>& data = input.second;
		int version;
		AuctionFileFormat fileFormat;
		std::unique_ptr<AuctionFile> auctionFile{new AuctionFile};

		item->setName(filename);
		log(LL_Info, "Deserializing file %s\n", filename.c_str());

		if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
			log(LL_Error, "Invalid file, unrecognized header signature: %s\n", filename.c_str());
			return EBADMSG;
		}

		if(!AuctionWriter::deserialize(&auctionFile->auctions, data)) {
			log(LL_Error, "Can't deserialize file %s\n", filename.c_str());
			return EILSEQ;
		}

		auctionFile->filename = std::move(filename);

		addResult(item, std::move(auctionFile));
	}

	return 0;
}

void P12DeserializeAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
