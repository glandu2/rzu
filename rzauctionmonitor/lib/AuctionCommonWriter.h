#ifndef AUCTIONCOMMONWRITER_H
#define AUCTIONCOMMONWRITER_H

#include "Core/Object.h"
#include <unordered_map>
#include <vector>
#include <array>
#include "Extern.h"
#include <stdint.h>
#include "AuctionFile.h"

class RZAUCTION_EXTERN AuctionCommonWriter : public Object {
	DECLARE_CLASSNAME(AuctionCommonWriter, 0)

public:
	AuctionCommonWriter(size_t categoryCount);

	void beginProcess();
	void endProcess();
	void beginCategory(size_t category, time_t time);
	void endCategory(size_t category, time_t time);
	void dumpAuctions(const std::string &auctionDir, const std::string &auctionFile, bool dumpDiff, bool dumpFull);
	void dumpAuctions(std::vector<uint8_t> &auctionData, bool doFulldump);

private:
	struct CategoryTime {
		time_t previousBegin;
		time_t begin;
		time_t end;

		CategoryTime() : previousBegin(0), begin(0), end(0) {}
		void resetTimes() {
			previousBegin = begin;
			begin = end = 0;
		}
	};

protected:
	void writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t>& data, time_t fileTimeStamp, const char* suffix);

	void adjustCategoryTimeRange(size_t category, time_t time);
	CategoryTime& getCategoryTime(size_t category);
	time_t getEstimatedPreviousCategoryBeginTime(size_t category);
	time_t getEstimatedCategoryBeginTime(size_t category);
	time_t getLastEndCategoryTime();
	void resetCategoryTime();
	size_t getCategoryNumber() { return categoryTime.size(); }

	void serializeHeader(AUCTION_HEADER& header, DumpType dumpType);
	void deserializeHeader(AUCTION_HEADER& header);

	virtual void resetAuctionProcess() = 0;
	virtual void serializeAuctionInfos(bool doFullDump, std::vector<uint8_t>& output) = 0;
	virtual void processRemainingAuctions() = 0;

private:
	static int compressGzip(std::vector<uint8_t>& compressedData, const std::vector<uint8_t> &sourceData, int level);

private:
	std::vector<uint8_t> fileData; //cache allocated memory
	int fileNumber;

	std::vector<CategoryTime> categoryTime;
};

#endif // AUCTIONSIMPLEDIFF_H
