#include "P1ReadAuction.h"

P1ReadAuction::P1ReadAuction()
    : PipelineStep<std::pair<std::string, std::string>,
                   std::pair<std::string, std::vector<AuctionWriter::file_data_byte>>>(10000, 10, 10),
      work(this, &P1ReadAuction::processWork, &P1ReadAuction::afterWork) {}

void P1ReadAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P1ReadAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<std::string, std::string>& filenames : sources) {
		std::string& filename = filenames.first;
		std::string fullFilename = std::move(filenames.second);
		std::vector<AuctionWriter::file_data_byte> data;

		item->setName(filename);
		log(LL_Info, "Reading file %s\n", filename.c_str());

		if(!AuctionWriter::readAuctionDataFromFile(fullFilename, data)) {
			log(LL_Error, "Cant read file %s\n", filename.c_str());
			return EIO;
		}

		if(!data.empty())
			addResult(item, std::make_pair(std::move(filename), std::move(data)));
	}

	return 0;
}

void P1ReadAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
