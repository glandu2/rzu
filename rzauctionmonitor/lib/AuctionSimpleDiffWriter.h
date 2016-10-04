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
#include "AuctionCommonWriter.h"

class RZAUCTION_EXTERN AuctionSimpleDiffWriter : public AuctionCommonWriter {
	DECLARE_CLASSNAME(AuctionSimpleDiffWriter, 0)

public:
	AuctionSimpleDiffWriter(size_t categoryCount);

	void addAuctionInfo(uint32_t uid, uint64_t time, uint16_t category, const uint8_t *data, size_t len);

	void importDump(AUCTION_SIMPLE_FILE* auctionFile);

private:
	virtual void resetAuctionProcess();
	virtual void serializeAuctionInfos(bool doFullDump, std::vector<uint8_t>& output);
	virtual void processRemainingAuctions();

private:
	template<typename T> static const AuctionSimpleData& getAuctionInfoFromValue(const std::pair<T, AuctionSimpleData>& val) { return val.second; }
	static const AuctionSimpleData& getAuctionInfoFromValue(const AuctionSimpleData& val) { return val; }

private:
	std::unordered_map<uint32_t, AuctionSimpleData> auctionsState;
};

#endif // AUCTIONSIMPLEDIFF_H
