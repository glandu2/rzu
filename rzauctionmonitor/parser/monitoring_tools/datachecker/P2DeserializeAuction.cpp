#include "P2DeserializeAuction.h"
#include <GameClient/TS_SC_AUCTION_SEARCH.h>

P2DeserializeAuction::P2DeserializeAuction()
    : PipelineStep<std::pair<std::string, std::vector<AuctionWriter::file_data_byte>>, void>(1000, 16, 10),
      work(this, &P2DeserializeAuction::processWork, &P2DeserializeAuction::afterWork) {}

void P2DeserializeAuction::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

bool tryDeserializeData(const std::vector<uint8_t>& data, uint32_t epic) {
	MessageBuffer structBuffer(data.data(), data.size(), epic);
	TS_SEARCHED_AUCTION_INFO item;

	item.deserialize(&structBuffer);
	if(!structBuffer.checkFinalSize()) {
		Object::logStatic(Object::LL_Error, "DB_Item", "Invalid item data, can't deserialize\n");
		return false;
	} else if(structBuffer.getParsedSize() != data.size()) {
		Object::logStatic(Object::LL_Warning,
		                  "DB_Item",
		                  "Invalid item data size, can't deserialize safely with epic 0x%" PRIx32 "\n",
		                  epic);
		return false;
	}

	return true;
}

int P2DeserializeAuction::processWork(std::shared_ptr<WorkItem> item) {
	auto sources = std::move(item->getSources());
	for(std::pair<std::string, std::vector<AuctionWriter::file_data_byte>>& input : sources) {
		std::string& filename = input.first;
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

		if(!auctionFile.auctions.empty() && !auctionFile.auctions.front().data.empty()) {
			if(auctionFile.auctions.front().epic != auctionFile.auctions.back().epic) {
				log(LL_Error,
				    "File %s has multiple auction with different epic: %x, %x\n",
				    filename.c_str(),
				    auctionFile.auctions.front().epic,
				    auctionFile.auctions.back().epic);
				return EBADF;
			}

			const AUCTION_SIMPLE_INFO& auctionInfo = auctionFile.auctions.front();
			uint32_t originalEpic = AuctionComplexDiffWriter::parseEpic(auctionInfo.epic, auctionInfo.time);
			uint32_t foundEpic = 0xFFFFFF;

			if(originalEpic != 0xFFFFFF) {
				// Find epic that makes data valid between epic N and epic N + 2 exluded
				for(uint32_t epic = originalEpic; epic < originalEpic + 0x000200;) {
					if(tryDeserializeData(auctionInfo.data, epic)) {
						foundEpic = epic;
						break;
					}

					epic++;
					if((epic & 0xFF) >= 0x10) {
						// there is no epic with subnumber above 0x10, skip them to next epic
						epic = (epic & 0xFFFFFF00) + 0x100;
					}
				}
			}

			if(foundEpic == 0xFFFFFF || foundEpic != originalEpic) {
				log(LL_Error,
				    "In auction file %s: Can't parse data with given epic %x (indata epic: %x) at timestamp %u, got "
				    "epic %x "
				    "instead (if 0xFFFFFF, no valid deserialization was found with tried epics)\n",
				    filename.c_str(),
				    originalEpic,
				    auctionInfo.epic,
				    (unsigned int) auctionInfo.time,
				    foundEpic);
				return EBADMSG;
			}
		}
		log(LL_Info, "Deserialization ok for %s\n", filename.c_str());
	}

	return 0;
}

void P2DeserializeAuction::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
