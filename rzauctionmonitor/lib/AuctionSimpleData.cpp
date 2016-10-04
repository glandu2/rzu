#include "AuctionSimpleData.h"
#include "Core/Utils.h"
#include "Core/Object.h"

AuctionSimpleData::AuctionSimpleData(uint32_t uid, uint64_t timeMin, uint64_t timeMax, uint16_t category, const uint8_t *data, size_t len)
    : processStatus(PS_Added),
      uid(uid),
      updateTime(timeMax),
      previousUpdateTime(timeMin),
      category(category)
{
	parseData(data, len);
}

bool AuctionSimpleData::update(uint64_t time, const uint8_t *data, size_t len)
{
	bool dataModified = parseData(data, len);
	if(dataModified && processStatus != PS_Added) {
		setStatus(PS_Updated, time);
	} else if(!dataModified && processStatus != PS_Added && processStatus != PS_Updated) {
		setStatus(PS_Unmodifed, time);
	}

	return dataModified;
}

void AuctionSimpleData::unmodified(uint64_t time)
{
	if(processStatus != PS_Added && processStatus != PS_Updated) {
		setStatus(PS_Unmodifed, time);
	}
}

void AuctionSimpleData::remove(uint64_t time)
{
	if(processStatus == PS_NotProcessed) {
		setStatus(PS_Deleted, time);
	}
}

void AuctionSimpleData::resetProcess()
{
	if(processStatus != PS_Deleted) {
		processStatus = PS_NotProcessed;

		if(updateTime)
			previousUpdateTime = updateTime;
		updateTime = 0;
	}
}

void AuctionSimpleData::setPreviousUpdateTime(uint64_t time)
{
	previousUpdateTime = time;
}

AuctionSimpleData AuctionSimpleData::createFromDump(AUCTION_SIMPLE_INFO *auctionInfo)
{
	AuctionSimpleData auction(auctionInfo->uid, auctionInfo->previousTime, auctionInfo->time, auctionInfo->category, auctionInfo->data.data(), auctionInfo->data.size());

	return auction;
}

void AuctionSimpleData::serialize(AUCTION_SIMPLE_INFO *auctionInfo) const
{
	auctionInfo->uid = uid;
	auctionInfo->previousTime = previousUpdateTime;
	auctionInfo->time = updateTime;
	auctionInfo->diffType = getAuctionDiffType();
	auctionInfo->category = category;

	auctionInfo->data = rawData;
}

DiffType AuctionSimpleData::getAuctionDiffType() const {
	switch(processStatus) {
		case PS_Deleted: return D_Deleted;
		case PS_Added: return D_Added;
		case PS_Updated: return D_Updated;
		case PS_Unmodifed: return D_Unmodified;
		case PS_NotProcessed: return D_Invalid;
	}
	return D_Invalid;
}

void AuctionSimpleData::setStatus(ProcessStatus status, uint64_t time)
{
	if(status == PS_Deleted && processStatus != PS_Deleted) {
		updateTime = time;
	} else if(status != PS_NotProcessed) {
		updateTime = time;
	}

	processStatus = status;
}

bool AuctionSimpleData::parseData(const uint8_t *data, size_t len)
{
	bool hasChanged = rawData.size() != len || memcmp(rawData.data(), data, len) != 0;

	rawData.assign(data, data + len);

	return hasChanged;
}
