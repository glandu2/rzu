#include "AuctionSimpleData.h"
#include "Core/Utils.h"
#include "Core/Object.h"

AuctionSimpleData::AuctionSimpleData(AuctionUid uid, uint64_t timeMin, uint64_t timeMax, uint16_t category, const uint8_t *data, size_t len)
    : IAuctionData(uid, category, timeMin, timeMax),
      processStatus(PS_Added)
{
	parseData(data, len);
}

bool AuctionSimpleData::doUpdate(uint64_t timeMax, const uint8_t *data, size_t len)
{
	bool dataModified = parseData(data, len);
	if(dataModified && processStatus != PS_Added) {
		setStatus(PS_Updated, timeMax);
	} else if(!dataModified && processStatus != PS_Added && processStatus != PS_Updated) {
		setStatus(PS_Unmodifed, timeMax);
	}

	return dataModified;
}

void AuctionSimpleData::beginProcess()
{
	if(processStatus == PS_Deleted) {
		log(LL_Error, "Start process with previously deleted auction: %d\n", getUid().get());
	} else {
		processStatus = PS_NotProcessed;

		advanceTime();
	}
}

void AuctionSimpleData::endProcess(uint64_t categoryEndTime)
{
	if(processStatus == PS_NotProcessed) {
		setStatus(PS_Deleted, categoryEndTime);
		log(LL_Debug, "Auction info removed: 0x%08X\n", getUid().get());
	}
}

bool AuctionSimpleData::isInFinalState() const
{
	return processStatus == PS_Deleted;
}

void AuctionSimpleData::setStatus(ProcessStatus status, uint64_t time)
{
	if(status == PS_Deleted && processStatus != PS_Deleted) {
		updateTime(time);
	} else if(status != PS_NotProcessed) {
		updateTime(time);
	}

	processStatus = status;
}

bool AuctionSimpleData::outputInPartialDump()
{
	DiffType diffType = getAuctionDiffType();

	if(diffType >= D_Invalid)
		logStatic(LL_Error, getStaticClassName(), "Invalid diff flag: %d for auction 0x%08X\n", diffType, getUid().get());

	if(diffType == D_Unmodified)
		return false;
	return true;
}

AuctionSimpleData *AuctionSimpleData::createFromDump(const AUCTION_SIMPLE_INFO *auctionInfo)
{
	AuctionSimpleData* auctionSimpleData;

	auctionSimpleData = new AuctionSimpleData(AuctionUid(auctionInfo->uid),
	                             auctionInfo->previousTime,
	                             auctionInfo->time,
	                             auctionInfo->category,
	                             auctionInfo->data.data(),
	                             auctionInfo->data.size());
	ProcessStatus processStatus = getAuctionProcessStatus((DiffType)auctionInfo->diffType);
	if(processStatus == PS_NotProcessed) {
		logStatic(LL_Error, getStaticClassName(), "Invalid diffType, can't convert to processStatus: %d, uid: 0x%08X\n", auctionInfo->diffType, auctionInfo->uid);
	} else {
		auctionSimpleData->processStatus = processStatus;
	}

	return auctionSimpleData;
}

void AuctionSimpleData::serialize(AUCTION_SIMPLE_INFO *auctionInfo) const
{
	auctionInfo->uid = getUid().get();
	auctionInfo->previousTime = getTimeMin();
	auctionInfo->time = getTimeMax();
	auctionInfo->diffType = getAuctionDiffType();
	auctionInfo->category = getCategory();

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
	logStatic(LL_Error, getStaticClassName(), "Invalid processStatus, can't convert to diffType: %d, uid: %d\n", processStatus, getUid().get());
	return D_Invalid;
}

AuctionSimpleData::ProcessStatus AuctionSimpleData::getAuctionProcessStatus(DiffType diffType) {
	switch(diffType) {
		case D_Deleted: return PS_Deleted;
		case D_Added: return PS_Added;
		case D_Updated: return PS_Updated;
		case D_Unmodified: return PS_Unmodifed;

		case D_Base:
		case D_MaybeDeleted:
		case D_Invalid: return PS_NotProcessed;
	}
	return PS_NotProcessed;
}


bool AuctionSimpleData::parseData(const uint8_t *data, size_t len)
{
	bool hasChanged = rawData.size() != len || memcmp(rawData.data(), data, len) != 0;

	rawData.assign(data, data + len);

	return hasChanged;
}
