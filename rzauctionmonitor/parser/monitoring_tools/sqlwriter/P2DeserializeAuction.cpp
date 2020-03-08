#include "P2DeserializeAuction.h"
#include "AuctionWriter.h"

P2DeserializeAuction::P2DeserializeAuction()
    : PipelineStep<std::pair<PipelineState, std::vector<uint8_t>>, std::pair<PipelineState, AUCTION_SIMPLE_FILE>>(
          100, 1, 10),
      work(this, &P2DeserializeAuction::processWork, &P2DeserializeAuction::afterWork) {}

void P2DeserializeAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P2DeserializeAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<PipelineState, std::vector<uint8_t>>& input : sources) {
		std::string& filename = input.first.lastFilenameParsed;
		const std::vector<uint8_t>& data = input.second;
		int version;
		AuctionFileFormat fileFormat;
		AUCTION_SIMPLE_FILE auctionFile;

		item->setName(filename);
		log(LL_Info, "Deserializing file %s\n", filename.c_str());

		if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
			log(LL_Error, "Invalid file, unrecognized header signature: %s\n", filename.c_str());
			return EBADMSG;
		}

		if(!AuctionWriter::deserialize(&auctionFile, data)) {
			log(LL_Error, "Can't deserialize file %s\n", filename.c_str());
			return EILSEQ;
		}

		addResult(item, std::make_pair(std::move(input.first), std::move(auctionFile)));
	}

	return 0;
}

void P2DeserializeAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
