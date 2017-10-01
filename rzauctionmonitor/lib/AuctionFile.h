#ifndef AUCTIONFILE_H
#define AUCTIONFILE_H

#include "AuctionSimpleFile.h"
#include "Packet/PacketDeclaration.h"

// clang-format off
#define AUCTION_INFO_DEF(_) \
	_(simple)  (uint32_t, uid) \
	_(simple)  (int64_t, time) \
	_(simple)  (int64_t, previousTime) \
	_(simple)  (bool, estimatedEndTimeFromAdded) \
	_(simple)  (int64_t, estimatedEndTimeMin) \
	_(simple)  (int64_t, estimatedEndTimeMax) \
	_(simple)  (uint16_t, diffType) \
	_(simple)  (uint16_t, category) \
	_(count)   (uint16_t, data) \
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
	_(count)   (uint32_t, auctions) \
	_(dynarray)(AUCTION_INFO, auctions)
CREATE_STRUCT(AUCTION_FILE);

#endif  // AUCTIONFILE_H
