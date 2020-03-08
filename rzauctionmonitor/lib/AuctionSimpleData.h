#ifndef AUCTIONSIMPLEDATA_H
#define AUCTIONSIMPLEDATA_H

#include "AuctionSimpleFile.h"
#include "AuctionUid.h"
#include "Extern.h"
#include "IAuctionData.h"
#include <stdint.h>
#include <vector>

struct AuctionSimpleDataView {
	AuctionUid uid;
	uint64_t timeMin;
	uint64_t timeMax;
	uint16_t category;
	const uint8_t* data;
	size_t dataSize;
};

class RZAUCTION_EXTERN AuctionSimpleData : public IAuctionData {
	DECLARE_CLASSNAME(AuctionSimpleData, 0)
private:
	enum ProcessStatus { PS_NotProcessed, PS_Unmodifed, PS_Added, PS_Updated, PS_Deleted };

public:
	AuctionSimpleData(AuctionUid uid,
	                  uint64_t timeMin,
	                  uint64_t timeMax,
	                  uint16_t category,
	                  uint32_t epic,
	                  const uint8_t* data,
	                  size_t len);

	virtual bool doUpdate(uint64_t timeMax, uint32_t epic, const uint8_t* data, size_t len);
	virtual void beginProcess();
	virtual void endProcess(uint64_t categoryEndTime);
	virtual bool isInFinalState() const;

	virtual bool outputInPartialDump();

	static AuctionSimpleData* createFromDump(const AUCTION_SIMPLE_INFO* auctionInfo);
	void serialize(AUCTION_SIMPLE_INFO* auctionInfo) const;

	void setStatus(ProcessStatus status, uint64_t time);

private:
	bool parseData(uint32_t epic, const uint8_t* data, size_t len);
	DiffType getAuctionDiffType() const;
	static ProcessStatus getAuctionProcessStatus(DiffType diffType);

private:
	ProcessStatus processStatus;
	uint32_t rawDataEpic;
	std::vector<uint8_t> rawData;
};

#endif
