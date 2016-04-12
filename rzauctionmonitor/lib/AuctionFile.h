#ifndef AUCTIONFILE_H
#define AUCTIONFILE_H

#include "Packet/PacketDeclaration.h"

enum DiffType {
	D_Added = 0,
	D_Updated = 1,
	D_Deleted = 2,
	D_Unmodified = 3,
	D_Base = 4,
	D_Invalid = 0xFFFF
};

enum DumpType {
	DT_Diff,
	DT_Full
};

#define AUCTION_CATEGORY_INFO_DEF(simple_, array_, dynarray_, count_, string_, dynstring_) \
	simple_(int64_t, beginTime) \
	simple_(int64_t, endTime)
CREATE_STRUCT(AUCTION_CATEGORY_INFO);

#define AUCTION_INFO_DEF(simple_, array_, dynarray_, count_, string_, dynstring_) \
	simple_  (uint32_t, uid) \
	simple_  (int64_t, time) \
	simple_  (int64_t, previousTime) \
	simple_  (uint16_t, diffType) \
	simple_  (uint16_t, category) \
	count_   (uint16_t, dataSize, data) \
	dynarray_(uint8_t, data)
CREATE_STRUCT(AUCTION_INFO);

#define AUCTION_FILE_DEF(simple_, array_, dynarray_, count_, string_, dynstring_) \
	array_   (char, signature, 4) \
	simple_  (uint32_t, file_version) \
	simple_  (int8_t, dumpType) \
	count_   (uint16_t, categoryNumber, categories) \
	dynarray_(AUCTION_CATEGORY_INFO, categories) \
	count_   (uint32_t, auctionNumber, auctions) \
	dynarray_(AUCTION_INFO, auctions)
CREATE_STRUCT(AUCTION_FILE);

#endif // AUCTIONFILE_H
