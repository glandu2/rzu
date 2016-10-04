#ifndef AUCTIONFILE_H
#define AUCTIONFILE_H

#include "Packet/PacketDeclaration.h"

#pragma pack(push, 1)
    struct AuctionDataEnd {
		int8_t duration_type;
		int64_t bid_price;
		int64_t price;
		char seller[31];
		int8_t bid_flag;
	};
#pragma pack(pop)

enum DiffType {
	D_Added = 0,
	D_Updated = 1,
	D_Deleted = 2,
	D_Unmodified = 3,
	D_Base = 4,
	D_MaybeDeleted = 5,
	D_Invalid = 0xFFFF
};

enum DumpType {
	DT_Diff,
	DT_Full
};

struct AuctionFileHeader {
	char signature[4];
	uint32_t file_version;
};

enum AuctionFileVersion {
	AUCTION_V3 = 3,
	AUCTION_V4 = 4,  // add auction meta data in struct format (like prices, seller)
	AUCTION_LATEST = AUCTION_V4
};

#define AUCTION_CATEGORY_INFO_DEF(_) \
	_(simple)(int64_t, previousBegin) \
	_(simple)(int64_t, beginTime) \
	_(simple)(int64_t, endTime)
CREATE_STRUCT(AUCTION_CATEGORY_INFO);

#define AUCTION_HEADER_DEF(_) \
	_(array)   (char, signature, 4) \
	_(simple)  (uint32_t, file_version) \
	_(simple)  (int8_t, dumpType) \
	_(count)   (uint16_t, categoryNumber, categories) \
	_(dynarray)(AUCTION_CATEGORY_INFO, categories)
CREATE_STRUCT(AUCTION_HEADER);

#define AUCTION_INFO_DEF(_) \
	_(simple)  (uint32_t, uid) \
	_(simple)  (int64_t, time) \
	_(simple)  (int64_t, previousTime) \
	_(simple)  (bool, estimatedEndTimeFromAdded) \
	_(simple)  (int64_t, estimatedEndTimeMin) \
	_(simple)  (int64_t, estimatedEndTimeMax) \
	_(simple)  (uint16_t, diffType) \
	_(simple)  (uint16_t, category) \
	_(count)   (uint16_t, dataSize, data) \
	_(dynarray)(uint8_t, data) \
	_(simple)  (int8_t, duration_type, version >= AUCTION_V4) \
	_(simple)  (int64_t, bid_price, version >= AUCTION_V4) \
	_(simple)  (int64_t, price, version >= AUCTION_V4) \
	_(string)  (seller, 31, version >= AUCTION_V4) \
	_(simple)  (int8_t, bid_flag, version >= AUCTION_V4) \
	_(simple)  (bool, deleted, version >= AUCTION_V4) \
	_(simple)  (uint8_t, deletedCount, version >= AUCTION_V4 && deleted, 0)
CREATE_STRUCT(AUCTION_INFO);

#define AUCTION_FILE_DEF(_) \
	_(simple)  (AUCTION_HEADER, header) \
	_(count)   (uint32_t, auctionNumber, auctions) \
	_(dynarray)(AUCTION_INFO, auctions)
CREATE_STRUCT(AUCTION_FILE);

#define AUCTION_SIMPLE_INFO_DEF(_) \
	_(simple)  (uint32_t, uid) \
	_(simple)  (int64_t, time) \
	_(simple)  (int64_t, previousTime) \
	_(simple)  (uint16_t, diffType) \
	_(simple)  (uint16_t, category) \
	_(count)   (uint16_t, dataSize, data) \
	_(dynarray)(uint8_t, data)
CREATE_STRUCT(AUCTION_SIMPLE_INFO);

#define AUCTION_SIMPLE_FILE_DEF(_) \
	_(simple)  (AUCTION_HEADER, header) \
	_(count)   (uint32_t, auctionNumber, auctions) \
	_(dynarray)(AUCTION_SIMPLE_INFO, auctions)
CREATE_STRUCT(AUCTION_SIMPLE_FILE);

#endif // AUCTIONFILE_H
