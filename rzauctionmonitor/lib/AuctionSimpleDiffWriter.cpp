#include "AuctionSimpleDiffWriter.h"
#include <time.h>

AuctionSimpleDiffWriter::AuctionSimpleDiffWriter(size_t categoryCount) : AuctionGenericWriter(categoryCount)
{
}

void AuctionSimpleDiffWriter::addAuctionInfo(AuctionUid uid, uint64_t time, uint16_t category, const uint8_t* data, size_t len)
{
	AuctionSimpleData* auctionData = getAuction(uid);
	if(auctionData)
		auctionData->doUpdate(time,
		                      data,
		                      len);
	else
		addAuction(new AuctionSimpleData(uid,
		                                 categoryTimeManager.getEstimatedPreviousCategoryBeginTime(category),
		                                 time,
		                                 category,
		                                 data,
		                                 len));
	categoryTimeManager.adjustCategoryTimeRange(category, time);
}

void AuctionSimpleDiffWriter::beginProcess()
{
	AuctionGenericWriter::beginProcess([](AuctionSimpleData* data) {
		data->beginProcess();
	});
}

void AuctionSimpleDiffWriter::endProcess()
{
	AuctionGenericWriter::endProcess([this](AuctionSimpleData* data) {
		data->endProcess(categoryTimeManager.getEstimatedCategoryEndTime(data->getCategory()));
	});
}
