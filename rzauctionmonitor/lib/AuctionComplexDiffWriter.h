#ifndef AUCTIONCOMPLEXDIFFWRITER_H
#define AUCTIONCOMPLEXDIFFWRITER_H

#include "Core/Object.h"
#include <vector>
#include "Extern.h"
#include <stdint.h>
#include "AuctionFile.h"
#include "AuctionComplexData.h"
#include "AuctionGenericWriter.h"

class RZAUCTION_EXTERN AuctionComplexDiffWriter : public AuctionGenericWriter<AuctionComplexData> {
	DECLARE_CLASSNAME(AuctionComplexDiffWriter, 0)

public:
	AuctionComplexDiffWriter(size_t categoryCount);

	void addAuctionInfo(const AUCTION_SIMPLE_INFO *auction);
	void beginProcess();
	void endProcess();
	void setDiffInputMode(bool diffMode);

	void dumpAuctions(std::vector<uint8_t> &output, bool doFullDump, bool alwaysWithData);
	AUCTION_FILE exportDump(bool doFullDump, bool alwaysWithData);
	void importDump(AUCTION_FILE *auctionFile);

private:
	bool diffMode;
};

#endif
