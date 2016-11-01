#ifndef AUCTIONCOMMONWRITER_H
#define AUCTIONCOMMONWRITER_H

#include "Core/Object.h"
#include <vector>
#include "Extern.h"
#include <stdint.h>
#include "AuctionFile.h"
#include "CategoryTimeManager.h"

class RZAUCTION_EXTERN AuctionCommonWriter : public Object {
	DECLARE_CLASSNAME(AuctionCommonWriter, 0)

public:
	AuctionCommonWriter(size_t categoryCount);

	void beginCategory(size_t category, time_t time) { categoryTimeManager.beginCategory(category, time); }
	void endCategory(size_t category, time_t time) { categoryTimeManager.endCategory(category, time); }

protected:
	void writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t>& data, time_t fileTimeStamp, const char* suffix);

	CategoryTimeManager categoryTimeManager;
private:
	static int compressGzip(std::vector<uint8_t>& compressedData, const std::vector<uint8_t> &sourceData, int level);

private:
	int fileNumber;
};

#endif // AUCTIONSIMPLEDIFF_H
