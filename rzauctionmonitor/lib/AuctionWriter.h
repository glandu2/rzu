#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Extern.h"
#include <stdint.h>
#include <string>
#include <vector>

enum AuctionFileFormat { AFF_Old, AFF_Simple, AFF_Complex };

class RZAUCTION_EXTERN AuctionWriter : public Object {
	DECLARE_CLASSNAME(AuctionWriter, 0)

	struct uninitialized_byte {
		uint8_t value;

		uninitialized_byte() noexcept {
			// do nothing
			static_assert(sizeof *this == sizeof value, "invalid size");
			// static_assert(__alignof uninitialized_byte == __alignof value, "invalid alignment");
		}
	};

	typedef uninitialized_byte file_data_byte;

public:
	static void writeAuctionDataToFile(std::string auctionsFile, const std::vector<uint8_t>& data);
	static bool readAuctionDataFromFile(std::string auctionsFile, std::vector<AuctionWriter::file_data_byte>& data);

	static bool getAuctionFileFormat(const std::vector<AuctionWriter::file_data_byte>& data,
	                                 int* version,
	                                 AuctionFileFormat* format);
	static bool deserialize(AUCTION_SIMPLE_FILE* file, const std::vector<file_data_byte>& data);
	static bool deserialize(AUCTION_FILE* file, const std::vector<file_data_byte>& data);

	static inline constexpr bool diffTypeInPartialDump(DiffType diffType) {
		return diffType != D_Unmodified && diffType != D_MaybeDeleted;
	}

private:
	template<class T> static bool deserializeFile(const std::vector<file_data_byte>& buffer, T* auctionFile);
	template<class AuctionHeader>
	static bool deserializeOldFile(const std::vector<file_data_byte>& data, AUCTION_SIMPLE_FILE* auctionFile);
	static int compressGzip(std::vector<uint8_t>& compressedData,
	                        const std::vector<uint8_t>& sourceData,
	                        int level,
	                        const char*& msg);
	static int uncompressGzip(std::vector<file_data_byte>& uncompressedData,
	                          const std::vector<uint8_t>& compressedData,
	                          const char*& msg);
};
