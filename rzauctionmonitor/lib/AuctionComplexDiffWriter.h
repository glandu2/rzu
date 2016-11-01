#ifndef AUCTIONCOMPLEXDIFF_H
#define AUCTIONCOMPLEXDIFF_H

#include "Core/Object.h"
#include <unordered_map>
#include <vector>
#include <array>
#include "Extern.h"
#include <stdint.h>
#include "AuctionFile.h"
#include "AuctionComplexData.h"
#include "AuctionGenericWriter.h"

class RZAUCTION_EXTERN AuctionComplexDiffWriter : public AuctionGenericWriter<AuctionComplexData, AUCTION_FILE> {
	DECLARE_CLASSNAME(AuctionComplexDiffWriter, 0)

public:
	AuctionComplexDiffWriter(size_t categoryCount);

	void addAuctionInfo(const AUCTION_SIMPLE_INFO *auction);
	void beginProcess();
	void endProcess();
	void setDiffInputMode(bool diffMode);
private:
	AuctionComplexDiffWriter(const AuctionComplexDiffWriter& other) = delete;
	void operator=(const AuctionComplexDiffWriter& other) = delete;
	bool diffMode;
};

#endif // AUCTIONSIMPLEDIFF_H
