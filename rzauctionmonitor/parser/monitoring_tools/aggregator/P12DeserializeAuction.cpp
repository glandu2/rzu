#include "P12DeserializeAuction.h"

P12DeserializeAuction::P12DeserializeAuction()
    : PipelineStep<std::pair<PipelineState, std::vector<AuctionWriter::file_data_byte>>,
                   std::pair<PipelineState, AUCTION_SIMPLE_FILE>>(10, 1, 1),
      work(this, &P12DeserializeAuction::processWork, &P12DeserializeAuction::afterWork) {}

void P12DeserializeAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P12DeserializeAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<PipelineState, std::vector<AuctionWriter::file_data_byte>>& input : sources) {
		const std::string& filename = input.first.lastFilenameParsed;
		const std::vector<AuctionWriter::file_data_byte>& data = input.second;
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

void P12DeserializeAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
