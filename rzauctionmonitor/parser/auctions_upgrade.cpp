#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <map>
#include <vector>
#include "AuctionComplexDiffWriter.h"
#include "AuctionFile.h"
#include <memory>
#include "AuctionSQLWriter.h"
#include "Core/EventLoop.h"
#include "Config/GlobalCoreConfig.h"
#include "Database/DbConnectionPool.h"
#include "Database/DbConnection.h"
#include "LibRzuInit.h"

#pragma pack(push, 1)

struct AuctionHeaderV0 {
	int32_t size;
	int64_t time;
	int32_t category;
	int32_t page;

	int getSize() { return size; }
	int16_t getVersion() { return 0; }
	int16_t getFlag() { return D_Base; }
	int64_t getTime() { return time; }
	int64_t getPreviousTime() { return 0; }
	int16_t getCategory() { return category; }
};

struct AuctionHeaderV1 {
	int32_t size;
	int16_t version;
	int16_t flag;
	int64_t time;
	int16_t category;

	int getSize() { return size - sizeof(*this); }
	int16_t getVersion() { return version; }
	int16_t getFlag() { return flag; }
	int64_t getTime() { return time; }
	int64_t getPreviousTime() { return 0; }
	int16_t getCategory() { return category; }
};

struct AuctionHeaderV2 {
	int32_t size;
	int16_t version;
	int16_t flag;
	int64_t time;
	int64_t previousTime;
	int16_t category;

	int getSize() { return size - sizeof(*this); }
	int16_t getVersion() { return version; }
	int16_t getFlag() { return flag; }
	int64_t getTime() { return time; }
	int64_t getPreviousTime() { return previousTime; }
	int16_t getCategory() { return category; }
};
#pragma pack(pop)

struct AuctionFile {
	size_t alreadyExistingAuctions;
	std::vector<AUCTION_SIMPLE_INFO> auctions;
	bool isFull;

	void addAuction(const AUCTION_SIMPLE_INFO& auctionData, AuctionComplexDiffWriter* auctionWriter) {
		if(auctionData.diffType != D_Added && auctionData.diffType != D_Base)
			isFull = false;
		if(auctionData.diffType == D_Added && auctionWriter->hasAuction(AuctionUid(auctionData.uid)))
			alreadyExistingAuctions++;
		auctions.push_back(auctionData);
	}
};

bool readAllFileData(std::vector<uint8_t>& buffer, FILE* file) {
	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer.resize(fileSize);
	size_t readDataSize = fread(buffer.data(), 1, fileSize, file);
	if(readDataSize != fileSize) {
		Object::logStatic(Object::LL_Error, "main", "Coulnd't read file data, size: %ld, read: %ld\n", (long int)fileSize, (long int)readDataSize);
		return false;
	}

	return true;
}

template<class T>
bool deserializeAuctions(const std::vector<uint8_t>& buffer, T* auctionFile) {
	struct Header {
		char sign[4];
		uint32_t version;
	};

	if(buffer.size() < sizeof(Header)) {
		Object::logStatic(Object::LL_Error, "main", "File size too small, can't deserialize\n");
		return false;
	}

	uint32_t version = ((Header*)buffer.data())->version;

	MessageBuffer structBuffer(buffer.data(), buffer.size(), version);

	auctionFile->deserialize(&structBuffer);
	if(!structBuffer.checkFinalSize()) {
		Object::logStatic(Object::LL_Error, "main", "Invalid file data, can't deserialize\n");
		return false;
	}

	return true;
}

template<class AuctionHeader>
bool readData(FILE* file, AUCTION_SIMPLE_INFO* auctionData) {
	AuctionHeader header;
	union Data {
		uint32_t uid;
		char buffer[4096];
	} data;

	size_t ret = fread(&header, sizeof(header), 1, file);
	if(ret != 1)
		return false;

	size_t dataSize = header.getSize();
	if(dataSize > sizeof(data.buffer)) {
		Object::logStatic(Object::LL_Error, "main", "Error: data size too large: %d at offset %d\n", dataSize, (int)(ftell(file) - sizeof(header)));
		return false;
	}

	if(fread(data.buffer, 1, dataSize, file) != dataSize)
		return false;

	auctionData->uid = data.uid;
	auctionData->diffType = header.getFlag();
	auctionData->time = header.getTime();
	auctionData->previousTime = header.getPreviousTime();
	auctionData->category = header.getCategory();

	auctionData->data = std::vector<uint8_t>(data.buffer, data.buffer + dataSize);

	return true;
}

bool readFile(FILE* file, AuctionFile* auctionFile, AuctionComplexDiffWriter* auctionWriter) {
	union FileHeader {
		struct OldHeader {
			uint32_t size;
			uint16_t version;
			uint16_t padding;
		} oldHeader;
		struct NewHeader {
			char signature[4];
			uint32_t version;
		} newHeader;
	};


	FileHeader fileHeader;
	if(fread(&fileHeader, 1, sizeof(fileHeader), file) != sizeof(fileHeader))
		return false;

	auctionFile->isFull = true;
	auctionFile->alreadyExistingAuctions = 0;

	if(strncmp(fileHeader.newHeader.signature, "RHS", 4) == 0) {
		AUCTION_SIMPLE_FILE auctionFileData;
		std::vector<uint8_t> buffer;
		if(!readAllFileData(buffer, file))
			return false;

		if(!deserializeAuctions(buffer, &auctionFileData))
			return false;

		for(size_t i = 0; i < auctionFileData.header.categories.size(); i++) {
			const AUCTION_CATEGORY_INFO& category = auctionFileData.header.categories[i];
			auctionWriter->beginCategory(i, category.beginTime);
			auctionWriter->endCategory(i, category.endTime);
		}

		for(size_t i = 0; i < auctionFileData.auctions.size(); i++) {
			const AUCTION_SIMPLE_INFO& auctionInfo = auctionFileData.auctions[i];
			auctionFile->addAuction(auctionInfo, auctionWriter);
		}
	} else if(strncmp(fileHeader.newHeader.signature, "RAH", 4) == 0) {
		AUCTION_FILE auctionFileData;
		std::vector<uint8_t> buffer;
		if(!readAllFileData(buffer, file))
			return false;

		if(!deserializeAuctions(buffer, &auctionFileData))
			return false;

		for(size_t i = 0; i < auctionFileData.auctions.size(); i++) {
			const AUCTION_INFO& auctionInfo = auctionFileData.auctions[i];
			AUCTION_SIMPLE_INFO auctionSimpleInfo;

			auctionSimpleInfo.uid = auctionInfo.uid;
			auctionSimpleInfo.time = auctionInfo.time;
			auctionSimpleInfo.previousTime = auctionInfo.previousTime;
			auctionSimpleInfo.diffType = auctionInfo.diffType;
			auctionSimpleInfo.category = auctionInfo.category;
			auctionSimpleInfo.data = auctionInfo.data;

			auctionFile->addAuction(auctionSimpleInfo, auctionWriter);
		}
	} else {
		bool result;
		AUCTION_SIMPLE_INFO auctionData;
		int trueVersion = fileHeader.oldHeader.version;

		if(trueVersion > 1000)
			trueVersion = 0;

		clearerr(file);
		fseek(file, 0, SEEK_SET);

		while(!feof(file)) {
			switch(trueVersion) {
				case 0:
					result = readData<AuctionHeaderV0>(file, &auctionData);
					break;

				case 1:
					result = readData<AuctionHeaderV1>(file, &auctionData);
					break;

				case 2:
					result = readData<AuctionHeaderV2>(file, &auctionData);
					break;

				default:
					Object::logStatic(Object::LL_Error, "main", "Unsupported version %d\n", trueVersion);
					return false;
			}

			if(!result) {
				if(!feof(file)) {
					Object::logStatic(Object::LL_Error, "main", "Failed to read file\n");
					return false;
				} else {
					return true;
				}
			}

			auctionFile->addAuction(auctionData, auctionWriter);
		}
	}

	if(auctionFile->alreadyExistingAuctions == 0)
		auctionFile->isFull = false;

	return true;
}

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
	char filename[1024];
	int fileNumber = 0;
	std::vector<DB_Item::Input> dbInputs;

	if(argc > 1 && argv[1][0] != '/' && argv[1][0] != '-') {
		Object::logStatic(Object::LL_Info, "main", "Loading state file %s\n", argv[1]);

		FILE* file = fopen(argv[1], "rb");
		if(!file) {
			Object::logStatic(Object::LL_Error, "main", "Cant open file %s\n", argv[1]);
			return 1;
		}

		AUCTION_FILE auctionFileData;
		std::vector<uint8_t> buffer;
		if(!readAllFileData(buffer, file))
			return 3;

		if(!deserializeAuctions(buffer, &auctionFileData))
			return 4;

		auctionWriter.importDump(&auctionFileData);
		fclose(file);
	}

	while(fgets(filename, sizeof(filename), stdin)) {
		if(filename[0] == '\n' || filename[0] == '\r')
			continue;
		filename[strcspn(filename, "\r\n")] = 0;

		//dump last processed file
//		if(fileNumber > 0)
//			auctionWriter.dumpAuctions("output", "auctions.bin", true, false, true);

		Object::logStatic(Object::LL_Info, "main", "Reading file %s\n", filename);

		FILE* file = fopen(filename, "rb");
		if(!file) {
			Object::logStatic(Object::LL_Error, "main", "Cant open file %s\n", filename);
			return 1;
		}


		AuctionFile auctionFile;

		if(!readFile(file, &auctionFile, &auctionWriter)) {
			Object::logStatic(Object::LL_Info, "main", "Failed to process file %s\n", filename);
			fclose(file);
			return 2;
		}

		Object::logStatic(Object::LL_Info, "main", "Processing file %s, detected type: %s, alreadyExistingAuctions: %d/%d\n",
		        filename,
		        auctionFile.isFull ? "full" : "diff",
		        auctionFile.alreadyExistingAuctions,
		        auctionWriter.getAuctionCount());

		auctionWriter.setDiffInputMode(!auctionFile.isFull);
		auctionWriter.beginProcess();

		for(size_t auction = 0; auction < auctionFile.auctions.size(); auction++) {
			const AUCTION_SIMPLE_INFO& auctionData = auctionFile.auctions[auction];
			auctionWriter.addAuctionInfo(&auctionData);
		}
		auctionWriter.endProcess();

		fclose(file);

		fileNumber++;

		AUCTION_FILE auctionFinalFile = auctionWriter.exportDump(false, true);
		for(const AUCTION_INFO& auctionInfo : auctionFinalFile.auctions) {
			DB_Item::addAuction(dbInputs, auctionInfo);
		}

		if(dbInputs.size() > 10000) {
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

	//dump last file with full
	//auctionWriter.dumpAuctions("output", "auctions.bin", true, true, true);

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", fileNumber);

	return 0;
}
