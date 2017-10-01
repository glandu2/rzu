#ifndef AUCTIONWRITER_H
#define AUCTIONWRITER_H

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Extern.h"
#include <stdint.h>
#include <string>
#include <vector>

enum AuctionFileFormat { AFF_Old, AFF_Simple, AFF_Complex };

class RZAUCTION_EXTERN AuctionWriter : public Object {
	DECLARE_CLASSNAME(AuctionWriter, 0)

public:
	static void writeAuctionDataToFile(std::string auctionsFile, const std::vector<uint8_t>& data);
	static bool readAuctionDataFromFile(std::string auctionsFile, std::vector<uint8_t>& data);

	static bool getAuctionFileFormat(const std::vector<uint8_t>& data, int* version, AuctionFileFormat* format);
	static bool deserialize(AUCTION_SIMPLE_FILE* file, const std::vector<uint8_t>& data);
	static bool deserialize(AUCTION_FILE* file, const std::vector<uint8_t>& data);

private:
	template<class T> static bool deserializeFile(const std::vector<uint8_t>& buffer, T* auctionFile);
	template<class AuctionHeader>
	static bool deserializeOldFile(const std::vector<uint8_t>& data, AUCTION_SIMPLE_FILE* auctionFile);
	static int compressGzip(std::vector<uint8_t>& compressedData,
	                        const std::vector<uint8_t>& sourceData,
	                        int level,
	                        const char*& msg);
	static int uncompressGzip(std::vector<uint8_t>& uncompressedData,
	                          const std::vector<uint8_t>& compressedData,
	                          const char*& msg);
};

#endif
