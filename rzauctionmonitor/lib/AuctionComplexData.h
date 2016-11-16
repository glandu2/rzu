#ifndef AUCTIONCOMPLEXDATA_H
#define AUCTIONCOMPLEXDATA_H

#include <stdint.h>
#include <vector>
#include "Extern.h"
#include "AuctionFile.h"
#include "IAuctionData.h"

class RZAUCTION_EXTERN AuctionComplexData : public IAuctionData {
	DECLARE_CLASSNAME(AuctionComplexData, 0)
public:
	enum ProcessStatus {
		PS_NotProcessed,
		PS_Unmodifed,
		PS_Added,
		PS_Updated,
		PS_MaybeDeleted,
		PS_Deleted
	};
	enum DurationType {
		DT_Unknown = 0,
		DT_Short = 1,  // 6h
		DT_Medium = 2, // 24h
		DT_Long = 3    // 72h
	};
	enum BidFlag {
		BF_Bidded = 0,
		BF_MyBid = 1,
		BF_NoBid = 2
	};
	struct DynamicData {
		DurationType durationType;
		int64_t bidPrice;
		BidFlag bidFlag;

		DynamicData() : durationType(DT_Unknown), bidPrice(0), bidFlag(BF_Bidded) {}

		bool operator==(const DynamicData& other) {
			return durationType == other.durationType &&
			        bidPrice == other.bidPrice &&
			        bidFlag == other.bidFlag;
		}

		bool operator!=(const DynamicData& other) {
			return !(*this == other);
		}
	};

public:
	AuctionComplexData(uint32_t uid, uint64_t timeMin, uint64_t timeMax, uint16_t category, const uint8_t *data, size_t len);

	bool update(uint64_t time, const uint8_t *data, size_t len);
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

	bool parseData(const uint8_t *data, size_t len);
	void postParseData(bool newData);
	uint32_t durationTypeToSecond(DurationType durationType);
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

	//updatable data
	std::vector<uint8_t> rawData;
	DynamicData dynamicData;
	DynamicData oldDynamicData;


	// computed
	bool estimatedEndTimeFromAdded;
	uint64_t estimatedEndTimeMin;
	uint64_t estimatedEndTimeMax;
};

#endif
