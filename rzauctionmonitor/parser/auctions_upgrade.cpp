#include "AuctionComplexDiffWriter.h"
#include "AuctionFile.h"
#include "AuctionSQLWriter.h"
#include "AuctionWriter.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Database/DbConnection.h"
#include "Database/DbConnectionPool.h"
#include "LibRzuInit.h"
#include <map>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <vector>

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

cval<std::string>& stateFile = CFG_CREATE("statefile", "");
int main(int argc, char* argv[]) {
	LibRzuInit();
	DbConnectionPool dbConnectionPool;
	DbBindingLoader::get()->initAll(&dbConnectionPool);

	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
	               GlobalCoreConfig::get()->log.level,
	               GlobalCoreConfig::get()->log.consoleLevel,
	               GlobalCoreConfig::get()->log.dir,
	               GlobalCoreConfig::get()->log.file,
	               GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	ConfigInfo::get()->dump();

	AuctionComplexDiffWriter auctionWriter(19);

	if(!DB_Item::createTable(&dbConnectionPool)) {
		Object::logStatic(Object::LL_Info, "main", "Failed to create table\n");
		// return 2;
	}

	std::string stateFileName = stateFile.get();
	if(!stateFileName.empty()) {
		Object::logStatic(Object::LL_Info, "main", "Loading state file %s\n", stateFileName.c_str());

		std::vector<uint8_t> data;

		if(AuctionWriter::readAuctionDataFromFile(stateFileName, data)) {
			AUCTION_FILE auctionFileData;
			if(!AuctionWriter::deserialize(&auctionFileData, data)) {
				Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", stateFileName.c_str());
				return 3;
			}

			auctionWriter.importDump(&auctionFileData);
		} else {
			Object::logStatic(Object::LL_Error, "main", "Cant read file %s\n", stateFileName.c_str());
		}
	}

	char filename[1024];
	int fileNumber = 0;
	std::vector<DB_Item::Input> dbInputs;

	while(fgets(filename, sizeof(filename), stdin)) {
		if(filename[0] == '\n' || filename[0] == '\r')
			continue;
		filename[strcspn(filename, "\r\n")] = 0;

		// dump last processed file
		//		if(fileNumber > 0)
		//			auctionWriter.dumpAuctions("output", "auctions.bin", true, false, true);

		Object::logStatic(Object::LL_Info, "main", "Reading file \"%s\"\n", filename);

		std::vector<uint8_t> data;
		int version;
		AuctionFileFormat fileFormat;
		AuctionFile auctionFile;

		if(!AuctionWriter::readAuctionDataFromFile(filename, data)) {
			Object::logStatic(Object::LL_Error, "main", "Cant read file %s\n", filename);
			return 1;
		}

		if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
			Object::logStatic(Object::LL_Error, "main", "Invalid file, unrecognized header signature: %s\n", filename);
			return 1;
		}

		if(!AuctionWriter::deserialize(&auctionFile.auctions, data)) {
			Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
			return 3;
		}

		auctionFile.adjustDetectedType(&auctionWriter);

		Object::logStatic(
		    Object::LL_Info,
		    "main",
		    "Processing file %s, detected type: %s, alreadyExistingAuctions: %d/%d, addedAuctionsInFile: %d/%d\n",
		    filename,
		    auctionFile.isFull ? "full" : "diff",
		    (int) auctionFile.alreadyExistingAuctions,
		    (int) auctionWriter.getAuctionCount(),
		    (int) auctionFile.addedAuctionsInFile,
		    (int) auctionWriter.getAuctionCount());

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

		fileNumber++;

		AUCTION_FILE auctionFinalFile = auctionWriter.exportDump(false, true);
		for(size_t i = 0; i < auctionFinalFile.auctions.size(); i++) {
			const AUCTION_INFO& auctionInfo = auctionFinalFile.auctions[i];
			DB_Item::addAuction(dbInputs, auctionInfo);
		}

		if(dbInputs.size() >= 10000) {
			DbQueryJob<DB_Item>::executeNoResult(dbInputs);
			EventLoop::getInstance()->run(UV_RUN_DEFAULT);
			dbInputs.clear();
		}
	}

	if(!dbInputs.empty()) {
		DbQueryJob<DB_Item>::executeNoResult(dbInputs);
		EventLoop::getInstance()->run(UV_RUN_DEFAULT);
		dbInputs.clear();
	}

	//	if(!stateFileName.empty()) {
	//		std::vector<uint8_t> data;
	//		auctionWriter.dumpAuctions(data, true, true);
	//		AuctionWriter::writeAuctionDataToFile(stateFileName, data);
	//	}

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", fileNumber);

	return 0;
}
