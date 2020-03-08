#ifndef AUCTIONSIMPLEDIFF_H
#define AUCTIONSIMPLEDIFF_H

#include "AuctionGenericWriter.h"
#include "AuctionSimpleData.h"
#include "AuctionSimpleFile.h"
#include "Core/Object.h"
#include "Extern.h"
#include <stdint.h>
#include <vector>

class RZAUCTION_EXTERN AuctionSimpleDiffWriter : public AuctionGenericWriter<AuctionSimpleData> {
	DECLARE_CLASSNAME(AuctionSimpleDiffWriter, 0)

public:
	AuctionSimpleDiffWriter(size_t categoryCount);

	void addAuctionInfo(
	    AuctionUid uid, uint64_t time, uint16_t category, uint32_t epic, const uint8_t* data, size_t len);
	void beginProcess();
	void endProcess();

	void dumpAuctions(std::vector<uint8_t>& output, bool doFullDump);
	AUCTION_SIMPLE_FILE exportDump(bool doFullDump);
	void importDump(const AUCTION_SIMPLE_FILE* auctionFile);
};

#endif  // AUCTIONSIMPLEDIFF_H
