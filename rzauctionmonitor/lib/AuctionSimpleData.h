#ifndef AUCTIONSIMPLEDATA_H
#define AUCTIONSIMPLEDATA_H

#include <stdint.h>
#include <vector>
#include "Extern.h"
#include "AuctionFile.h"

class RZAUCTION_EXTERN AuctionSimpleData {
public:
	enum ProcessStatus {
		PS_NotProcessed,
		PS_Unmodifed,
		PS_Added,
		PS_Updated,
		PS_Deleted
	};

public:
	AuctionSimpleData(uint32_t uid, uint64_t timeMin, uint64_t timeMax, uint16_t category, const uint8_t *data, size_t len);
	bool update(uint64_t time, const uint8_t *data, size_t len);
	void unmodified(uint64_t time);
	void remove(uint64_t time);
	void resetProcess();
	void setPreviousUpdateTime(uint64_t time);

	static AuctionSimpleData createFromDump(AUCTION_SIMPLE_INFO* auctionInfo);
	void serialize(AUCTION_SIMPLE_INFO* auctionInfo) const;

	DiffType getAuctionDiffType() const;

	ProcessStatus getProcessStatus() const { return processStatus; }
	uint32_t getUid() const { return uid; }
	uint64_t getUpdateTime() const { return updateTime; }
	uint64_t getPreviousUpdateTime() const { return previousUpdateTime; }
	int32_t getCategory() const { return category; }
	std::vector<uint8_t> getRawData() const { return rawData; }

protected:
	bool parseData(const uint8_t *data, size_t len);
	void setStatus(ProcessStatus status, uint64_t time);

public:
	// status in current processing loop
	ProcessStatus processStatus;

	// key
	uint32_t uid;

	// data time validity range
	uint64_t updateTime;
	uint64_t previousUpdateTime;

	// fixed data
	int32_t category;

	//updatable data
	std::vector<uint8_t> rawData;
};

#endif
