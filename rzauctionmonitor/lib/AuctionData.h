#ifndef AUCTIONINFO_H
#define AUCTIONINFO_H

#include <stdint.h>
#include <vector>
#include "Extern.h"
#include "AuctionFile.h"

class RZAUCTION_EXTERN AuctionInfo {
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
	AuctionInfo(uint32_t uid, uint64_t timeMin, uint64_t timeMax, uint16_t category, const uint8_t *data, size_t len);
	bool update(uint64_t time, const uint8_t *data, size_t len);
	void unmodified(uint64_t time);
	void remove(uint64_t time);
	void maybeStillDeleted();
	void resetProcess();
	void setPreviousUpdateTime(uint64_t time);

	static AuctionInfo createFromDump(AUCTION_INFO* auctionInfo);
	void serialize(AUCTION_INFO* auctionInfo) const;

	DiffType getAuctionDiffType() const;

	ProcessStatus getProcessStatus() const { return processStatus; }
	bool getDeleted() const  { return deleted; }
	uint32_t getDeletedCount() const  { return deletedCount; }
	uint32_t getUid() const { return uid; }
	uint64_t getUpdateTime() const { return updateTime; }
	uint64_t getPreviousUpdateTime() const { return previousUpdateTime; }
	int32_t getCategory() const { return category; }
	std::string getSeller() const { return seller; }
	int64_t getPrice() const { return price; }
	std::vector<uint8_t> getRawData() const { return rawData; }
	DurationType getDurationType() const { return dynamicData.durationType; }
	int64_t getBidPrice() const { return dynamicData.bidPrice; }
	BidFlag getBidFlag() const { return dynamicData.bidFlag; }
	uint64_t getEstimatedEndTimeMin() const { return estimatedEndTimeMin; }
	uint64_t getEstimatedEndTimeMax() const { return estimatedEndTimeMax; }

protected:
	bool parseData(const uint8_t *data, size_t len);
	void postParseData(bool newData);
	uint32_t durationTypeToSecond(DurationType durationType);
	void setStatus(ProcessStatus status, uint64_t time);

public:
	// status in current processing loop
	ProcessStatus processStatus;

	bool deleted;
	int deletedCount;

	// key
	uint32_t uid;

	// data time validity range
	uint64_t updateTime;
	uint64_t previousUpdateTime;

	// fixed data
	int32_t category;
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
