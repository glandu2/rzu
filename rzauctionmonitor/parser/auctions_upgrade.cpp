#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <map>
#include <vector>
#include "AuctionWriter.h"
#include "AuctionFile.h"
#include <memory>

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
	struct AuctionData {
		struct AuctionHeader {
			int64_t time;
			int64_t previousTime;
			uint32_t uid;
			int16_t diffType;
			int16_t category;
			uint8_t deletedCount;
		};

		AuctionHeader header;
		std::vector<uint8_t> data;
	};

	size_t alreadyExistingAuctions;
	std::vector<AuctionData> auctions;
	bool isFull;

	void addAuction(AuctionData& auctionData, AuctionWriter* auctionWriter) {
		if(auctionData.header.diffType != D_Added && auctionData.header.diffType != D_Base)
			isFull = false;
		if(auctionData.header.diffType == D_Added && auctionWriter->hasAuction(auctionData.header.uid))
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

bool deserializeAuctions(const std::vector<uint8_t>& buffer, AUCTION_FILE* auctionFile) {
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
bool readData(FILE* file, AuctionFile::AuctionData* auctionData) {
	AuctionHeader header;
	union Data {
		uint32_t uid;
		char buffer[1024];
	} data;

	int ret = fread(&header, sizeof(header), 1, file);
	if(ret != 1)
		return false;

	int dataSize = header.getSize();
	if(dataSize > sizeof(data.buffer)) {
		Object::logStatic(Object::LL_Error, "main", "Error: data size too large: %d at offset %d\n", dataSize, (int)(ftell(file) - sizeof(header)));
		return false;
	}

	if(fread(data.buffer, 1, dataSize, file) != dataSize)
		return false;

	auctionData->header.uid = data.uid;
	auctionData->header.diffType = header.getFlag();
	auctionData->header.time = header.getTime();
	auctionData->header.previousTime = header.getPreviousTime();
	auctionData->header.category = header.getCategory();

	auctionData->data = std::vector<uint8_t>(data.buffer, data.buffer + dataSize);

	return true;
}

bool readFile(FILE* file, AuctionFile* auctionFile, AuctionWriter* auctionWriter) {
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

	if(strncmp(fileHeader.newHeader.signature, "RAH", 4) == 0) {
		AUCTION_FILE auctionFileData;
		std::vector<uint8_t> buffer;
		if(!readAllFileData(buffer, file))
			return false;

		if(!deserializeAuctions(buffer, &auctionFileData))
			return false;

		for(size_t i = 0; i < auctionFileData.auctions.size(); i++) {
			const AUCTION_INFO& auctionInfo = auctionFileData.auctions[i];
			AuctionFile::AuctionData auctionData;

			auctionData.header.uid = auctionInfo.uid;
			auctionData.header.diffType = auctionInfo.diffType;
			auctionData.header.time = auctionInfo.time;
			auctionData.header.previousTime = auctionInfo.previousTime;
			auctionData.header.category = auctionInfo.category;
			auctionData.header.deletedCount = auctionInfo.deletedCount;
			auctionData.data = auctionInfo.data;

			auctionFile->addAuction(auctionData, auctionWriter);
		}
	} else {
		bool result;
		AuctionFile::AuctionData auctionData;
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
	AuctionWriter auctionWriter(19);
	char filename[1024];
	int fileNumber = 0;

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

		auctionWriter.dumpAuctions("output", "auctions.bin", false, true);
	}

	while(fgets(filename, sizeof(filename), stdin)) {
		if(filename[0] == '\n' || filename[0] == '\r')
			continue;
		filename[strcspn(filename, "\r\n")] = 0;

		//dump last processed file
		if(fileNumber > 0)
			auctionWriter.dumpAuctions("output", "auctions.bin", true, false);

		FILE* file = fopen(filename, "rb");
		if(!file) {
			Object::logStatic(Object::LL_Error, "main", "Cant open file %s\n", filename);
			return 1;
		}

		typedef AuctionHeaderV2 AuctionHeader;

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

		for(size_t auction = 0; auction < auctionFile.auctions.size(); auction++) {
			AuctionFile::AuctionData& auctionData = auctionFile.auctions[auction];
			DiffType diffType = (DiffType)auctionData.header.diffType;
			uint32_t uid = *(uint32_t*)auctionData.data.data();
			if(auctionFile.isFull) {
				if(diffType != D_MaybeDeleted) {
					auctionWriter.addAuctionInfo(uid,
					                             auctionData.header.time,
					                             auctionData.header.category,
					                             auctionData.data.data(),
					                             auctionData.data.size());
				} else {
					auctionWriter.addMaybeDeletedAuctionInfo(uid,
					                                         auctionData.header.time,
					                                         0,
					                                         0,
					                                         auctionData.header.category,
					                                         auctionData.data.data(),
					                                         auctionData.data.size());
				}
			} else {
				auctionWriter.addAuctionInfoDiff(uid,
				                                 auctionData.header.time,
				                                 auctionData.header.previousTime,
				                                 diffType,
				                                 auctionData.header.category,
												 auctionData.data.data(),
												 auctionData.data.size());
			}
		}

		fclose(file);

		fileNumber++;
	}

	//dump last file with full
	auctionWriter.dumpAuctions("output", "auctions.bin", true, true);

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", fileNumber);

	return 0;
}
