#include "AuctionData.h"
#include "Core/Utils.h"
#include "Core/Object.h"

AuctionInfo::AuctionInfo(uint32_t uid, uint64_t timeMin, uint64_t timeMax, uint16_t category, const uint8_t *data, size_t len)
    : processStatus(PS_Added),
      deleted(false),
      deletedCount(0),
      uid(uid),
      updateTime(timeMax),
      previousUpdateTime(timeMin),
      category(category),
      price(0),
      estimatedEndTimeFromAdded(true),
      estimatedEndTimeMin(0),
      estimatedEndTimeMax(0)
{
	parseData(data, len);
	postParseData(true);
}

bool AuctionInfo::update(uint64_t time, const uint8_t *data, size_t len)
{
	bool dataModified = parseData(data, len);
	if(dataModified && processStatus != PS_Added) {
		setStatus(PS_Updated, time);
	} else if(!dataModified && processStatus != PS_Added && processStatus != PS_Updated) {
		setStatus(PS_Unmodifed, time);
	}

	if(dataModified)
		postParseData(false);

	return dataModified;
}

void AuctionInfo::unmodified(uint64_t time)
{
	if(processStatus != PS_Added && processStatus != PS_Updated) {
		setStatus(PS_Unmodifed, time);
	}
}

void AuctionInfo::remove(uint64_t time)
{
	if(processStatus == PS_NotProcessed) {
		setStatus(PS_MaybeDeleted, time);
	}
}

void AuctionInfo::maybeStillDeleted()
{
	if(deleted) {
		deletedCount++;

		if(deletedCount > 3)
			setStatus(PS_Deleted, updateTime);
		else if(processStatus == PS_NotProcessed)
			setStatus(PS_MaybeDeleted, updateTime);
	}
}

void AuctionInfo::resetProcess()
{
	processStatus = PS_NotProcessed;

	if(!deleted) {
		if(updateTime)
			previousUpdateTime = updateTime;
		updateTime = 0;
		oldDynamicData = dynamicData;
	}
}

void AuctionInfo::setPreviousUpdateTime(uint64_t time)
{
	previousUpdateTime = time;
}

AuctionInfo AuctionInfo::createFromDump(AUCTION_INFO *auctionInfo)
{
	AuctionInfo auction(auctionInfo->uid, auctionInfo->previousTime, auctionInfo->time, auctionInfo->category, auctionInfo->data.data(), auctionInfo->data.size());

	auction.deleted = auctionInfo->deleted;
	auction.deletedCount = auctionInfo->deletedCount;
	auction.seller = auctionInfo->seller;
	auction.price = auctionInfo->price;
	auction.dynamicData.bidFlag = (BidFlag) auctionInfo->bid_flag;
	auction.dynamicData.bidPrice = auctionInfo->bid_price;
	auction.dynamicData.durationType = (DurationType) auctionInfo->duration_type;
	auction.estimatedEndTimeFromAdded = auctionInfo->estimatedEndTimeFromAdded;
	auction.estimatedEndTimeMin = auctionInfo->estimatedEndTimeMin;
	auction.estimatedEndTimeMax = auctionInfo->estimatedEndTimeMax;

	return auction;
}

void AuctionInfo::serialize(AUCTION_INFO *auctionInfo, bool alwaysWithData) const
{
	auctionInfo->uid = uid;
	auctionInfo->previousTime = previousUpdateTime;
	auctionInfo->time = updateTime;
	auctionInfo->diffType = getAuctionDiffType();
	auctionInfo->category = category;
	auctionInfo->deleted = deleted;
	auctionInfo->deletedCount = deletedCount;
	auctionInfo->seller = seller;
	auctionInfo->price = price;
	auctionInfo->bid_flag = dynamicData.bidFlag;
	auctionInfo->bid_price = dynamicData.bidPrice;
	auctionInfo->duration_type = dynamicData.durationType;
	auctionInfo->estimatedEndTimeFromAdded = estimatedEndTimeFromAdded;
	auctionInfo->estimatedEndTimeMin = estimatedEndTimeMin;
	auctionInfo->estimatedEndTimeMax = estimatedEndTimeMax;

	if(alwaysWithData || auctionInfo->diffType == D_Added)
		auctionInfo->data = rawData;
}

DiffType AuctionInfo::getAuctionDiffType() const {
	switch(processStatus) {
		case PS_Deleted: return D_Deleted;
		case PS_Added: return D_Added;
		case PS_Updated: return D_Updated;
		case PS_Unmodifed: return D_Unmodified;
		case PS_MaybeDeleted: return D_MaybeDeleted;
		case PS_NotProcessed: return D_Invalid;
	}
	return D_Invalid;
}

void AuctionInfo::setStatus(ProcessStatus status, uint64_t time)
{
	if(status == PS_MaybeDeleted || status == PS_Deleted) {
		if(!deleted)
			updateTime = time;

		deleted = true;
	} else if(status != PS_NotProcessed) {
		deleted = false;
		deletedCount = 0;

		updateTime = time;
	}

	processStatus = status;
}

bool AuctionInfo::parseData(const uint8_t *data, size_t len)
{
	bool hasChanged;

	const AuctionDataEnd* auctionDataEnd = reinterpret_cast<const AuctionDataEnd*>(data + len - sizeof(AuctionDataEnd));

	if(auctionDataEnd->bid_flag > BF_NoBid || auctionDataEnd->duration_type < DT_Short || auctionDataEnd->duration_type > DT_Long) {
		Object::logStatic(Object::LL_Error, "AuctionInfo", "Auction data contains invalid values: bid flag = %d, duration_type = %d\n",
		                  auctionDataEnd->bid_flag, auctionDataEnd->duration_type);
		hasChanged = rawData.size() != len || memcmp(rawData.data(), data, len) != 0;

		rawData.assign(data, data + len);

		return hasChanged;
	}

	rawData.assign(data, data + len);

	seller = Utils::convertToString(auctionDataEnd->seller, sizeof(auctionDataEnd->seller) - 1);
	price = auctionDataEnd->price;


	DurationType durationType = static_cast<DurationType>(auctionDataEnd->duration_type);
	BidFlag bidFlag = static_cast<BidFlag>(auctionDataEnd->bid_flag);
	int64_t bidPrice = auctionDataEnd->bid_price;

	this->dynamicData.durationType = durationType;
	this->dynamicData.bidPrice = bidPrice;
	this->dynamicData.bidFlag = bidFlag;

	hasChanged = this->dynamicData != this->oldDynamicData;

	return hasChanged;
}

void AuctionInfo::postParseData(bool newData)
{
	DurationType oldDurationType = this->oldDynamicData.durationType;
	DurationType newDurationType = this->dynamicData.durationType;

	if(newDurationType != DT_Unknown) {
		uint32_t durationSecond = durationTypeToSecond(newDurationType);
		uint64_t endMin;
		uint64_t endMax;

		if(durationSecond) {
			endMin = durationSecond + previousUpdateTime;
			endMax = durationSecond + updateTime;

			if(!newData && oldDurationType != DT_Unknown && newDurationType != oldDurationType) {
				// Override estimation from added time, to prevent issue when item disappear from AH because of pages and item shifting between searches
				// Not applicable to endTimeMax because we are always sure of the presence of the item (when it is returned in search result)
				if(endMin > estimatedEndTimeMin || estimatedEndTimeFromAdded)
					estimatedEndTimeMin = endMin;
				if(endMax < estimatedEndTimeMax)
					estimatedEndTimeMax = endMax;
				estimatedEndTimeFromAdded = false;
			} else if(newData && estimatedEndTimeFromAdded) {
				estimatedEndTimeMin = endMin;
				estimatedEndTimeMax = endMax;
			}
		}
	}
}

uint32_t AuctionInfo::durationTypeToSecond(DurationType durationType)
{
	switch(durationType) {
		case DT_Long: return 72 * 3600;
		case DT_Medium: return 24 * 3600;
		case DT_Short: return 6 * 3600;
		case DT_Unknown: return 0;
	}

	return 0;
}
