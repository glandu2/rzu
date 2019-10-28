#ifndef AUCTIONSIMPLEFILE_H
#define AUCTIONSIMPLEFILE_H

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

enum DurationType {
	DT_Unknown = 0,
	DT_Short = 1,   // 6h
	DT_Medium = 2,  // 24h
	DT_Long = 3     // 72h
};
enum BidFlag { BF_Bidded = 0, BF_MyBid = 1, BF_NoBid = 2 };

enum DiffType {
	D_Added = 0,
	D_Updated = 1,
	D_Deleted = 2,
	D_Unmodified = 3,
	D_Base = 4,
	D_MaybeDeleted = 5,
	D_Invalid = 0xFFFF
};

enum DumpType { DT_Diff, DT_Full, DT_UnknownDumpType };

struct AuctionFileHeader {
	char signature[4];
	uint32_t file_version;
};

enum AuctionFileVersion {
	AUCTION_V3 = 3,
	AUCTION_V4 = 4,  // add auction meta data in struct format (like prices, seller)
	AUCTION_V5 = 5,  // fix category time (as serialized after endProcess, must have previous times only)
	AUCTION_V6 = 6,  // add epic to be able to deserialize data with the right version
	AUCTION_VERSION_NUM,
	AUCTION_LATEST = AUCTION_VERSION_NUM - 1
};

// clang-format off
#define AUCTION_CATEGORY_INFO_DEF(_) \
	_(simple)(int64_t, previousBegin, version < AUCTION_V5) \
	_(simple)(int64_t, beginTime) \
	_(simple)(int64_t, endTime)
CREATE_STRUCT(AUCTION_CATEGORY_INFO);

#define AUCTION_HEADER_DEF(_) \
	_(array)   (char, signature, 4) \
	_(simple)  (uint32_t, file_version) \
	_(simple)  (int8_t, dumpType) \
	_(count)   (uint16_t, categories) \
	_(dynarray)(AUCTION_CATEGORY_INFO, categories)
CREATE_STRUCT(AUCTION_HEADER);

#define AUCTION_SIMPLE_INFO_DEF(_) \
	_(simple)  (uint32_t, uid) \
	_(simple)  (int64_t, time) \
	_(simple)  (int64_t, previousTime) \
	_(simple)  (uint16_t, diffType) \
	_(simple)  (uint16_t, category) \
	_(simple)  (uint32_t, epic, version >= AUCTION_V6, 0xFFFFFF) \
	_(count)   (uint16_t, data) \
	_(dynarray)(uint8_t, data)
CREATE_STRUCT(AUCTION_SIMPLE_INFO);

#define AUCTION_SIMPLE_FILE_DEF(_) \
	_(simple)  (AUCTION_HEADER, header) \
	_(count)   (uint32_t, auctions) \
	_(dynarray)(AUCTION_SIMPLE_INFO, auctions)
CREATE_STRUCT(AUCTION_SIMPLE_FILE);

#endif  // AUCTIONSIMPLEFILE_H
