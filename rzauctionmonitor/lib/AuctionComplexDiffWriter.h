#ifndef AUCTIONCOMPLEXDIFFWRITER_H
#define AUCTIONCOMPLEXDIFFWRITER_H

#include "AuctionComplexData.h"
#include "AuctionFile.h"
#include "AuctionGenericWriter.h"
#include "Core/Object.h"
#include "Extern.h"
#include <stdint.h>
#include <vector>

class RZAUCTION_EXTERN AuctionComplexDiffWriter : public AuctionGenericWriter<AuctionComplexData> {
	DECLARE_CLASSNAME(AuctionComplexDiffWriter, 0)

public:
	AuctionComplexDiffWriter(size_t categoryCount);

	void addAuctionInfo(const AUCTION_SIMPLE_INFO* auction);
	void beginProcess();
	void endProcess();
	void setDiffInputMode(bool diffMode);

	void dumpAuctions(std::vector<uint8_t>& output, bool doFullDump, bool alwaysWithData);
	AUCTION_FILE exportDump(bool doFullDump, bool alwaysWithData);
	void importDump(const AUCTION_FILE* auctionFile);

private:
	uint32_t parseEpic(uint32_t inputEpic, int64_t timestamp);

private:
	bool diffMode;
};

#endif
