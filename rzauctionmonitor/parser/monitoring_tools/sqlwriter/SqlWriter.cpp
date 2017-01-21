#include "SqlWriter.h"
#include "AuctionFile.h"
#include "Packet/JSONWriter.h"
#include "AuctionWriter.h"
#include "Core/Utils.h"
#include "GlobalConfig.h"
#include "AuctionComplexDiffWriter.h"

#define SQLWRITER_STATE_VERSION 0

#define SQLWRITER_STATE_DEF(_) \
	_(simple)  (uint16_t, file_version) \
	_(count)   (uint8_t, lastParsedFileSize, lastParsedFile) \
	_(dynstring)(lastParsedFile, false)
CREATE_STRUCT(SQLWRITER_STATE);


SqlWriter::SqlWriter()
{
}

void SqlWriter::exportState(std::string filename, const std::string& lastParsedFile)
{
	SQLWRITER_STATE sqlWriterState;

	log(LL_Debug, "Writting sqlwriter state file %s\n", filename.c_str());

	sqlWriterState.file_version = SQLWRITER_STATE_VERSION;
	sqlWriterState.lastParsedFile = lastParsedFile;

	int version = sqlWriterState.file_version;
	std::vector<uint8_t> data;
	data.resize(sqlWriterState.getSize(version));

	MessageBuffer buffer(data.data(), data.size(), version);
	sqlWriterState.serialize(&buffer);
	if(buffer.checkFinalSize() == false) {
		log(LL_Error, "Wrong buffer size, size: %d, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
	} else {
		AuctionWriter::writeAuctionDataToFile(filename, data);
	}

	if(!dbInputs.empty()) {
		flushDbInputs();
	}
}

void SqlWriter::importState(std::string filename, std::string& lastParsedFile)
{
	log(LL_Info, "Loading sqlwriter state file %s\n", filename.c_str());

	std::vector<uint8_t> data;

	if(AuctionWriter::readAuctionDataFromFile(filename, data)) {
		SQLWRITER_STATE aggregationState;
		uint16_t version = *reinterpret_cast<const uint16_t*>(data.data());
		MessageBuffer buffer(data.data(), data.size(), version);
		aggregationState.deserialize(&buffer);
		if(buffer.checkFinalSize() == false) {
			log(LL_Error, "Wrong buffer size, size: %d, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
			return;
		}

		lastParsedFile = aggregationState.lastParsedFile;
	} else {
		log(LL_Warning, "Cant read state file %s\n", filename.c_str());
	}
}

void SqlWriter::flushDbInputs()
{
	log(LL_Info, "Flushing %d auctions to database\n", (int)dbInputs.size());
	DbQueryJob<DB_Item>::executeNoResult(dbInputs);
	//EventLoop::getInstance()->run(UV_RUN_DEFAULT);
	dbInputs.clear();
}

bool SqlWriter::parseAuctions(AuctionComplexDiffWriter* auctionWriter) {
	AUCTION_FILE auctionFile = auctionWriter->exportDump(false, true);

	for(size_t i = 0; i < auctionFile.auctions.size(); i++) {
		const AUCTION_INFO& auctionInfo = auctionFile.auctions[i];
		DB_Item::addAuction(dbInputs, auctionInfo);
	}

	if(dbInputs.size() >= 10000) {
		flushDbInputs();
	}

	return true;
}

