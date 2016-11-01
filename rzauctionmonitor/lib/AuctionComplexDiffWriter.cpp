#include "AuctionComplexDiffWriter.h"
#include "Core/PrintfFormats.h"
#include <time.h>

AuctionComplexDiffWriter::AuctionComplexDiffWriter(size_t categoryCount) : AuctionGenericWriter(categoryCount), diffMode(false)
{
}

void AuctionComplexDiffWriter::addAuctionInfo(const AUCTION_SIMPLE_INFO* auction)
{
	if(auction->category >= categoryTimeManager.getCategoryNumber()) {
		log(LL_Error, "Auction diff with invalid category: uid: 0x%08X, diffType: %d, category: %d, category number: %d\n",
		    auction->uid, auction->diffType, auction->category, categoryTimeManager.getCategoryNumber());
		return;
	}

	categoryTimeManager.adjustCategoryTimeRange(auction->category, auction->time);

	DiffType diffType = (DiffType)(auction->diffType);
	uint64_t estimatedTruePreviousTime = 0;
	AuctionComplexData* auctionInfo = getAuction(AuctionUid(auction->uid));

	if(auction->previousTime) {
		estimatedTruePreviousTime = auction->previousTime;
	} else if(auctionInfo && (time_t)auctionInfo->getTimeMin() <= categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category)) {
		// auction previous time before category begin time, don't trust it and set previous time
		estimatedTruePreviousTime = categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category);
	}

	switch(diffType) {
		case D_Base:
		case D_Added:
		case D_Updated:
		{
			if(auctionInfo) {
				if(diffType == D_Added && auctionInfo->getProcessStatus() != AuctionComplexData::PS_MaybeDeleted) {
					log(LL_Error, "Added auction already exists: 0x%08X, with state: %d\n", auction->uid, auctionInfo->getProcessStatus());
				}

				if(estimatedTruePreviousTime) {
					auctionInfo->setTimeMin(estimatedTruePreviousTime);
				}

				auctionInfo->update(auction->time, auction->data.data(), auction->data.size());
			} else {
				if(diffType == D_Updated) {
					log(LL_Error, "Updated auction not found: 0x%08X\n", auction->uid);
				} else if(auction->previousTime && auction->previousTime != categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category)) {
					log(LL_Info, "Added auction previous time mismatch: category previous begin time: %" PRId64 ", new auction previous time: %" PRId64 "\n",
					    (int64_t)categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category), (int64_t)auction->previousTime);
				}

				uint64_t previousTimeToUse = auction->previousTime ? auction->previousTime : categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category);
				addAuction(new AuctionComplexData(auction->uid, previousTimeToUse, auction->time, auction->category, auction->data.data(), auction->data.size()));
			}
			break;
		}
		case D_Deleted: {
			if(!auctionInfo) {
				log(LL_Error, "Deleted auction not found: 0x%08X\n", auction->uid);
			} else {
				if(auctionInfo->getProcessStatus() != AuctionComplexData::PS_NotProcessed) {
					log(LL_Error, "Deleted auction is not in PS_NotProcessed flag: uid: 0x%08X, flag: %d\n", auction->uid, auctionInfo->getProcessStatus());
				}
/*
				if(categoryTimeManager.getCategoryTime(auction->category).end == 0) {
					log(LL_Info, "Category %d end time is 0, setting to deletion time %" PRIu64 "\n", auction->category, auction->time);
					categoryTimeManager.getCategoryTime(auction->category).end = auction->time;
				} else if(categoryTimeManager.getCategoryTime(auction->category).end != auction->time) {
					log(LL_Warning, "Deleted auction time is not category end time: uid: 0x%08X, time: %" PRIu64 ", category: %d, category end time: %" PRIu64 "\n",
						auction->uid, auction->time, auction->category, categoryTimeManager.getCategoryTime(auction->category).end);
				}*/

				if(estimatedTruePreviousTime) {
					auctionInfo->setTimeMin(estimatedTruePreviousTime);
				}

				auctionInfo->remove(auction->time);
			}
			break;
		}
		default:
			log(LL_Error, "Error: unsupported flag %d for auction 0x%08X\n", diffType, auction->uid);
			break;
	}
}


void AuctionComplexDiffWriter::beginProcess()
{
	AuctionGenericWriter::beginProcess([](AuctionComplexData* data) {
		data->beginProcess();
	});
}

void AuctionComplexDiffWriter::endProcess()
{
	AuctionGenericWriter::endProcess([this](AuctionComplexData* data) {
		data->endProcess(categoryTimeManager.getEstimatedCategoryBeginTime(data->getCategory()),
		                 categoryTimeManager.getEstimatedCategoryEndTime(data->getCategory()),
		                 diffMode);
	});
}

void AuctionComplexDiffWriter::setDiffInputMode(bool diffMode)
{
	this->diffMode = diffMode;
}
