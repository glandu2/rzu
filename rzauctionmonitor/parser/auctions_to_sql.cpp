#include "AuctionFile.h"
#include "AuctionSQLWriter.h"
#include "AuctionWriter.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/CharsetConverter.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "Database/DbConnection.h"
#include "Database/DbConnectionPool.h"
#include "Database/DbQueryJobRef.h"
#include "LibRzuInit.h"
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

template<class AUCTION_FILE> void writeSql(const AUCTION_FILE& auctionFile, std::vector<DB_Item::Input>& inputs) {
	for(size_t i = 0; i < auctionFile.auctions.size(); i++) {
		DB_Item::addAuction(inputs, auctionFile.auctions[i]);
	}
}

int main(int argc, char* argv[]) {
	LibRzuScopedUse useLibRzu;
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

	if(argc < 2) {
		Object::logStatic(Object::LL_Info, "main", "Usage: %s auctions.bin\n", argv[0]);
		return 1;
	}

	if(!DB_Item::createTable(&dbConnectionPool)) {
		Object::logStatic(Object::LL_Info, "main", "Failed to create table\n");
		return 2;
	}

	std::vector<DB_Item::Input> inputs;

	size_t processedFiles = 0;
	for(int i = 1; i < argc; i++) {
		const char* filename = argv[i];

		if(filename[0] == '/')
			continue;
		std::vector<uint8_t> data;
		int version;
		AuctionFileFormat fileFormat;

		if(!AuctionWriter::readAuctionDataFromFile(filename, data)) {
			Object::logStatic(Object::LL_Error, "main", "Cant read file %s\n", filename);
			return 1;
		}

		if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
			Object::logStatic(Object::LL_Error, "main", "Invalid file, unrecognized header signature: %s\n", filename);
			return 1;
		}

		if(fileFormat == AFF_Complex) {
			AUCTION_FILE file;
			if(!AuctionWriter::deserialize(&file, data)) {
				Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
				return 3;
			}
			writeSql(file, inputs);
		} else {
			AUCTION_SIMPLE_FILE file;
			if(!AuctionWriter::deserialize(&file, data)) {
				Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
				return 3;
			}
			writeSql(file, inputs);
		}

		processedFiles++;
	}

	DbQueryJob<DB_Item>::executeNoResult(inputs);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", (int) processedFiles);

	return 0;
}
