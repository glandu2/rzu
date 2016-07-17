#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <map>
#include <vector>
#include "AuctionWriter.h"
#include <memory>

#pragma pack(push, 1)

struct AuctionHeaderV0 {
	int32_t size;
	int64_t time;
	int32_t category;
	int32_t page;

	int getSize() { return size; }
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
	int16_t getFlag() { return flag; }
	int64_t getTime() { return time; }
	int64_t getPreviousTime() { return previousTime; }
	int16_t getCategory() { return category; }
};
#pragma pack(pop)

template<class T>
struct AuctionFile {
	struct AuctionData {
		T header;
		std::vector<uint8_t> data;
	};

	bool isFull;
	std::vector<AuctionData> auctions;
};

int main() {
	AuctionWriter auctionWriter(19);
	char filename[1024];
	int fileNumber = 0;

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

		AuctionFile<AuctionHeader> auctionFile;
		auctionFile.isFull = true;
		int alreadyExistingAuctions = 0;

		while(!feof(file)) {
			AuctionHeader header;
			union Data {
				int32_t uid;
				char buffer[1024];
			} data;

			if(fread(&header, sizeof(header), 1, file) != 1)
				break;

			int dataSize = header.getSize();
			if(dataSize > sizeof(data.buffer)) {
				Object::logStatic(Object::LL_Error, "main", "Error: data size too large: %d at offset %d\n", dataSize, (int)(ftell(file) - sizeof(header)));
				break;
			}

			if(fread(data.buffer, 1, dataSize, file) != dataSize)
				break;

			AuctionFile<AuctionHeader>::AuctionData auctionData;
			auctionData.header = header;
			auctionData.data = std::vector<uint8_t>(data.buffer, data.buffer + dataSize);

			auctionFile.auctions.push_back(auctionData);

			if(header.getFlag() != D_Added && header.getFlag() != D_Base)
				auctionFile.isFull = false;
			if(header.getFlag() == D_Added && auctionWriter.hasAuction(data.uid))
				alreadyExistingAuctions++;
		}

		if(alreadyExistingAuctions == 0)
			auctionFile.isFull = false;

		Object::logStatic(Object::LL_Info, "main", "Processing file %s, detected type: %s, alreadyExistingAuctions: %d/%d\n",
				filename,
				auctionFile.isFull ? "full" : "diff",
				alreadyExistingAuctions,
				auctionWriter.getAuctionCount());

		auctionWriter.setDiffInputMode(!auctionFile.isFull);

		for(size_t auction = 0; auction < auctionFile.auctions.size(); auction++) {
			AuctionFile<AuctionHeader>::AuctionData& auctionData = auctionFile.auctions[auction];
			DiffType diffType = (DiffType)auctionData.header.getFlag();
			uint32_t uid = *(uint32_t*)auctionData.data.data();
			if(auctionFile.isFull)
				auctionWriter.addAuctionInfo(uid,
											 auctionData.header.getTime(),
											 auctionData.header.getCategory(),
											 auctionData.data.data(),
											 auctionData.data.size());
			else
				auctionWriter.addAuctionInfoDiff(uid,
												 auctionData.header.getTime(),
												 auctionData.header.getPreviousTime(),
												 diffType,
												 auctionData.header.getCategory(),
												 auctionData.data.data(),
												 auctionData.data.size());
		}

		fclose(file);

		fileNumber++;
	}

	//dump last file with full
	auctionWriter.dumpAuctions("output", "auctions.bin", true, true);

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", fileNumber);

	return 0;
}
