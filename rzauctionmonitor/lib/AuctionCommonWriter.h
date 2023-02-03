#pragma once

#include "CategoryTimeManager.h"
#include "Core/Object.h"
#include "Extern.h"
#include <stdint.h>
#include <string>
#include <time.h>
#include <vector>

class RZAUCTION_EXTERN AuctionCommonWriter : public Object {
	DECLARE_CLASSNAME(AuctionCommonWriter, 0)

public:
	AuctionCommonWriter(size_t categoryCount);

	void beginCategory(size_t category, time_t time) { categoryTimeManager.beginCategory(category, time); }
	void endCategory(size_t category, time_t time) { categoryTimeManager.endCategory(category, time); }
	time_t getLastEndCategoryTime() { return categoryTimeManager.getLastEndCategoryTime(); }
	CategoryTimeManager& getCategoryTimeManager() { return categoryTimeManager; }

	void writeAuctionDataToFile(std::string auctionsDir,
	                            std::string auctionsFile,
	                            const std::vector<uint8_t>& data,
	                            time_t fileTimeStamp,
	                            const char* suffix);
	void writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t>& data);

	bool readAuctionDataFromFile(std::string auctionsDir, std::string auctionsFile, std::vector<uint8_t>& data);

private:
	static int compressGzip(std::vector<uint8_t>& compressedData,
	                        const std::vector<uint8_t>& sourceData,
	                        int level,
	                        const char*& msg);
	static int uncompressGzip(std::vector<uint8_t>& uncompressedData,
	                          const std::vector<uint8_t>& compressedData,
	                          const char*& msg);

protected:
	CategoryTimeManager categoryTimeManager;

private:
	int fileNumber;
};

