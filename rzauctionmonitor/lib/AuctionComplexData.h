#pragma once

#include "AuctionFile.h"
#include "Extern.h"
#include "IAuctionData.h"
#include <stdint.h>
#include <vector>

class RZAUCTION_EXTERN AuctionComplexData : public IAuctionData {
	DECLARE_CLASSNAME(AuctionComplexData, 0)
public:
	enum ProcessStatus { PS_NotProcessed, PS_Unmodifed, PS_Added, PS_Updated, PS_MaybeDeleted, PS_Deleted };
	struct DynamicData {
		DurationType durationType;
		int64_t bidPrice;
		BidFlag bidFlag;

		DynamicData() : durationType(DT_Unknown), bidPrice(0), bidFlag(BF_Bidded) {}

		bool operator==(const DynamicData& other) {
			return durationType == other.durationType && bidPrice == other.bidPrice && bidFlag == other.bidFlag;
		}

		bool operator!=(const DynamicData& other) { return !(*this == other); }
	};

public:
	AuctionComplexData(uint32_t uid,
	                   uint64_t timeMin,
	                   uint64_t timeMax,
	                   uint16_t category,
	                   uint32_t epic,
	                   const uint8_t* data,
	                   size_t len,
	                   bool ignoreNoData = false);

	bool update(uint64_t time, uint32_t epic, const uint8_t* data, size_t len);
	void remove(uint64_t time);

	virtual void beginProcess();
	virtual void endProcess(uint64_t categoryBeginTime, uint64_t categoryEndTime, bool diffMode);
	virtual bool outputInPartialDump();
	virtual bool isInFinalState() const;

	static AuctionComplexData* createFromDump(const AUCTION_INFO* auctionInfo);
	void serialize(AUCTION_INFO* auctionInfo, bool alwaysWithData) const;

	ProcessStatus getProcessStatus() const { return processStatus; }
	bool isDeleted() const { return deleted; }

protected:
	void unmodified(uint64_t time);
	void maybeStillDeleted();

	bool parseData(uint32_t epic, const uint8_t* data, size_t len);
	void postParseData(bool newData);
	uint32_t durationTypeToSecond(DurationType durationType, uint32_t epic);
	void setStatus(ProcessStatus status, uint64_t time);

	DiffType getAuctionDiffType() const;
	static ProcessStatus getAuctionProcessStatus(DiffType diffType);

private:
	// status in current processing loop
	ProcessStatus processStatus;

	bool deleted;
	int deletedCount;

	// fixed data
	std::string seller;
	int64_t price;

	// updatable data
	uint32_t rawDataEpic;
	std::vector<uint8_t> rawData;
	DynamicData dynamicData;
	DynamicData oldDynamicData;

	// computed
	bool estimatedEndTimeFromAdded;
	uint64_t estimatedEndTimeMin;
	uint64_t estimatedEndTimeMax;
};
