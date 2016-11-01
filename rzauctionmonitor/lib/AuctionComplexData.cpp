#include "AuctionComplexData.h"
#include "Core/Utils.h"
#include "Core/Object.h"
#include "Core/PrintfFormats.h"
#include "CategoryTimeManager.h"

AuctionComplexData::AuctionComplexData(uint32_t uid, uint64_t timeMin, uint64_t timeMax, uint16_t category, const uint8_t *data, size_t len)
    : IAuctionData(AuctionUid(uid), category, timeMin, timeMax),
      processStatus(PS_Added),
      deleted(false),
      deletedCount(0),
      price(0),
      estimatedEndTimeFromAdded(true),
      estimatedEndTimeMin(0),
      estimatedEndTimeMax(0)
{
	parseData(data, len);
	postParseData(true);
}

bool AuctionComplexData::update(uint64_t time, const uint8_t *data, size_t len)
{
	bool dataModified = parseData(data, len);
	if(dataModified && processStatus != PS_Added) {
		setStatus(PS_Updated, time);
	} else if(!dataModified && processStatus != PS_Added && processStatus != PS_Updated) {
		setStatus(PS_Unmodifed, time);
	}

	if(dataModified) {
		postParseData(false);
		log(LL_Debug, "Auction info modified: 0x%08X\n", getUid().get());
	}

	return dataModified;
}

void AuctionComplexData::unmodified(uint64_t time)
{
	if(processStatus != PS_Added && processStatus != PS_Updated) {
		setStatus(PS_Unmodifed, time);
	}
}

void AuctionComplexData::remove(uint64_t time)
{
	if(processStatus == PS_NotProcessed) {
		setStatus(PS_MaybeDeleted, time);
		maybeStillDeleted();
	}
}

void AuctionComplexData::maybeStillDeleted()
{
	if(deleted) {
		deletedCount++;

		if(deletedCount > 3)
			setStatus(PS_Deleted, getTimeMax());
		else if(processStatus == PS_NotProcessed)
			setStatus(PS_MaybeDeleted, getTimeMax());
	}
}

void AuctionComplexData::beginProcess()
{
	if(processStatus == PS_Deleted) {
		log(LL_Error, "Start process with previously deleted auction: %d\n", getUid().get());
	} else {
		processStatus = PS_NotProcessed;

		if(!deleted) {
			if(getTimeMax())
				advanceTime();
			oldDynamicData = dynamicData;
		}
	}
}

void AuctionComplexData::endProcess(uint64_t categoryBeginTime, uint64_t categoryEndTime, bool diffMode)
{
	if(processStatus == PS_NotProcessed) {
		if(!diffMode)
			remove(categoryEndTime);
		else if(!deleted)
			unmodified(categoryBeginTime);
		else
			maybeStillDeleted();
	}
}

bool AuctionComplexData::outputInPartialDump()
{
	DiffType diffType = getAuctionDiffType();

	if(diffType >= D_Invalid)
		logStatic(LL_Error, getStaticClassName(), "Invalid diff flag: %d for auction 0x%08X\n", diffType, getUid().get());

	if(diffType == D_Unmodified)
		return false;
	return true;
}

bool AuctionComplexData::isInFinalState() const
{
	return processStatus == PS_Deleted;
}

AuctionComplexData* AuctionComplexData::createFromDump(AUCTION_INFO *auctionInfo)
{
	AuctionComplexData* auction = new AuctionComplexData(auctionInfo->uid, auctionInfo->previousTime, auctionInfo->time, auctionInfo->category, auctionInfo->data.data(), auctionInfo->data.size());

	auction->deleted = auctionInfo->deleted;
	auction->deletedCount = auctionInfo->deletedCount;
	auction->seller = auctionInfo->seller;
	auction->price = auctionInfo->price;
	auction->dynamicData.bidFlag = (BidFlag) auctionInfo->bid_flag;
	auction->dynamicData.bidPrice = auctionInfo->bid_price;
	auction->dynamicData.durationType = (DurationType) auctionInfo->duration_type;
	auction->estimatedEndTimeFromAdded = auctionInfo->estimatedEndTimeFromAdded;
	auction->estimatedEndTimeMin = auctionInfo->estimatedEndTimeMin;
	auction->estimatedEndTimeMax = auctionInfo->estimatedEndTimeMax;

	return auction;
}

void AuctionComplexData::serialize(AUCTION_INFO *auctionInfo, bool alwaysWithData) const
{
	auctionInfo->uid = getUid().get();
	auctionInfo->previousTime = getTimeMin();
	auctionInfo->time = getTimeMax();
	auctionInfo->diffType = getAuctionDiffType();
	auctionInfo->category = getCategory();
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

DiffType AuctionComplexData::getAuctionDiffType() const {
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

void AuctionComplexData::setStatus(ProcessStatus status, uint64_t time)
{
	if(status == PS_MaybeDeleted || status == PS_Deleted) {
		if(!deleted)
			updateTime(time);

		deleted = true;
	} else if(status != PS_NotProcessed) {
		deleted = false;
		deletedCount = 0;

		updateTime(time);
	}

	processStatus = status;
}

bool AuctionComplexData::parseData(const uint8_t *data, size_t len)
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

void AuctionComplexData::postParseData(bool newData)
{
	DurationType oldDurationType = this->oldDynamicData.durationType;
	DurationType newDurationType = this->dynamicData.durationType;

	if(newDurationType != DT_Unknown) {
		uint32_t durationSecond = durationTypeToSecond(newDurationType);
		uint64_t endMin;
		uint64_t endMax;

		if(durationSecond) {
			endMin = durationSecond + getTimeMin();
			endMax = durationSecond + getTimeMax();

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

uint32_t AuctionComplexData::durationTypeToSecond(DurationType durationType)
{
	switch(durationType) {
		case DT_Long: return 72 * 3600;
		case DT_Medium: return 24 * 3600;
		case DT_Short: return 6 * 3600;
		case DT_Unknown: return 0;
	}

	return 0;
}
