#ifndef AUCTIONSIMPLEDIFF_H
#define AUCTIONSIMPLEDIFF_H

#include "Core/Object.h"
#include <unordered_map>
#include <vector>
#include <array>
#include "Extern.h"
#include <stdint.h>
#include "AuctionFile.h"
#include "AuctionSimpleData.h"
#include "AuctionGenericWriter.h"

class RZAUCTION_EXTERN AuctionSimpleDiffWriter : public AuctionGenericWriter<AuctionSimpleData, AUCTION_SIMPLE_FILE> {
	DECLARE_CLASSNAME(AuctionSimpleDiffWriter, 0)

public:
	AuctionSimpleDiffWriter(size_t categoryCount);

	void addAuctionInfo(AuctionUid uid, uint64_t time, uint16_t category, const uint8_t *data, size_t len);
	void beginProcess();
	void endProcess();
};

#endif // AUCTIONSIMPLEDIFF_H
