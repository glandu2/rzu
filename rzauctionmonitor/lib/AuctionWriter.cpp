#include "AuctionWriter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include <time.h>
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include <algorithm>
#include "Packet/MessageBuffer.h"

AuctionWriter::AuctionWriter(size_t categoryCount) : AuctionCommonWriter(categoryCount), diffMode(false)
{
}

void AuctionWriter::setDiffInputMode(bool diffMode)
{
	this->diffMode = diffMode;
}

void AuctionWriter::addAuctionInfo(uint32_t uid, uint64_t time, uint16_t category, const uint8_t* data, size_t len)
{
	if(diffMode) {
		log(LL_Error, "Called addAuctionInfo while in diff mode ! uid: 0x%08X\n", uid);
	}

	auto it = auctionsState.find(uid);
	if(it != auctionsState.end()) {
		AuctionInfo& auctionInfo = it->second;

		if(auctionInfo.update(time, data, len))
			log(LL_Debug, "Auction info modified: 0x%08X\n", uid);
	} else {
		auctionsState.insert(std::make_pair(uid,
		                                    AuctionInfo(uid,
		                                                getEstimatedPreviousCategoryBeginTime(category),
		                                                time,
														category,
		                                                data, len)));
		log(LL_Debug, "Auction info added: %d\n", uid);
	}

	adjustCategoryTimeRange(category, time);
}

void AuctionWriter::addMaybeDeletedAuctionInfo(uint32_t uid, uint64_t time, uint64_t previousTime, uint32_t deletedCount, uint16_t category, const uint8_t* data, size_t len)
{
	addAuctionInfo(uid, time, category, data, len);

	auto it = auctionsState.find(uid);
	if(it != auctionsState.end()) {
		AuctionInfo& auctionInfo = it->second;

		auctionInfo.setPreviousUpdateTime(previousTime);
		auctionInfo.remove(time);
		auctionInfo.deletedCount = (deletedCount > 0) ? (deletedCount - 1) : 0;
	} else {
		log(LL_Error, "Couln't add maybe deleted auction: uid: 0x%08X\n", uid);
	}
}

void AuctionWriter::addAuctionInfoDiff(uint32_t uid, uint64_t time, uint64_t previousTime, DiffType diffType, uint16_t category, const uint8_t *data, size_t len)
{
	if(!diffMode) {
		log(LL_Error, "Called addAuctionInfo while not in diff mode ! uid: 0x%08X\n", uid);
	}

	if(category >= getCategoryNumber()) {
		log(LL_Error, "Auction diff with invalid category: uid: 0x%08X, diffType: %d, category: %d, category number: %d\n",
		    uid, diffType, category, getCategoryNumber());
		return;
	}

	adjustCategoryTimeRange(category, time);

	if(diffType == D_Base)
		diffType = D_Added;

	switch(diffType) {
		case D_Added:
		case D_Updated:
		{
			auto it = auctionsState.find(uid);
			if(it != auctionsState.end()) {
				AuctionInfo& auctionInfo = it->second;

				if(diffType == D_Added && auctionInfo.processStatus != AuctionInfo::PS_MaybeDeleted) {
					log(LL_Error, "Added auction already exists: 0x%08X, with state: %d\n", uid, auctionInfo.processStatus);
				}

				if(previousTime) {
					auctionInfo.setPreviousUpdateTime(previousTime);
				} else if((time_t)auctionInfo.getPreviousUpdateTime() <= getEstimatedPreviousCategoryBeginTime(category)) {
					// auction previous time before category begin time, don't trust it and set previous time
					auctionInfo.setPreviousUpdateTime(getEstimatedPreviousCategoryBeginTime(category));
				}

				auctionInfo.update(time, data, len);
			} else {
				if(diffType == D_Updated) {
					log(LL_Error, "Updated auction not found: 0x%08X\n", uid);
				} else if(previousTime && previousTime != getEstimatedPreviousCategoryBeginTime(category)) {
					log(LL_Info, "Added auction previous time mismatch: category previous begin time: %" PRIu64 ", new auction previous time: %" PRIu64 "\n",
						getEstimatedPreviousCategoryBeginTime(category), previousTime);
				}

				uint64_t previousTimeToUse = previousTime ? previousTime : getEstimatedPreviousCategoryBeginTime(category);
				auto insertIt = auctionsState.insert(std::make_pair(uid, AuctionInfo(uid, previousTimeToUse, time, category, data, len)));
				if(insertIt.second == false) {
					log(LL_Error, "Coulnd't insert added auction: 0x%08X\n", uid);
				}
			}
			break;
		}
		case D_Deleted: {
			auto it = auctionsState.find(uid);
			if(it == auctionsState.end()) {
				log(LL_Error, "Deleted auction not found: 0x%08X\n", uid);
			} else {
				AuctionInfo& auctionInfo = it->second;
				if(auctionInfo.processStatus != AuctionInfo::PS_NotProcessed) {
					log(LL_Error, "Deleted auction is not in PS_NotProcessed flag: uid: 0x%08X, flag: %d\n", uid, auctionInfo.processStatus);
				}

				if(getCategoryTime(category).end == 0) {
					log(LL_Info, "Category %d end time is 0, setting to deletion time %" PRIu64 "\n", category, time);
					getCategoryTime(category).end = time;
				} else if(getCategoryTime(category).end != time) {
					log(LL_Warning, "Deleted auction time is not category end time: uid: 0x%08X, time: %" PRIu64 ", category: %d, category end time: %" PRIu64 "\n",
						uid, time, category, getCategoryTime(category).end);
				}

				if(previousTime) {
					auctionInfo.setPreviousUpdateTime(previousTime);
				} else if((time_t)auctionInfo.getPreviousUpdateTime() <= getEstimatedPreviousCategoryBeginTime(category)) {
					// auction previous time before category begin time, don't trust it and set previous time
					auctionInfo.setPreviousUpdateTime(getEstimatedPreviousCategoryBeginTime(category));
				}

				auctionInfo.remove(time);
				auctionInfo.maybeStillDeleted();
			}
			break;
		}
		default:
			log(LL_Error, "Error: unsupported flag %d for auction 0x%08X\n", diffType, uid);
			break;
	}

}

bool AuctionWriter::hasAuction(uint32_t uid)
{
	auto it = auctionsState.find(uid);
	if(it == auctionsState.end())
		return false;
	return true;
}

void AuctionWriter::processRemainingAuctions() {
	auto it = auctionsState.begin();
	for(; it != auctionsState.end(); ++it) {
		AuctionInfo& auctionInfo = it->second;
		if(auctionInfo.processStatus == AuctionInfo::PS_NotProcessed) {
			if(diffMode == false) {
				time_t endCategory = getCategoryTime(auctionInfo.category).end;
				if(endCategory == 0)
					log(LL_Warning, "End of category %d is time 0\n", auctionInfo.category);

				auctionInfo.remove(endCategory);
				log(LL_Debug, "Auction info removed: 0x%08X\n", auctionInfo.uid);
			} else if(!auctionInfo.deleted) {
				time_t beginCategory = getEstimatedCategoryBeginTime(auctionInfo.category);
				auctionInfo.unmodified(beginCategory);
			}

			auctionInfo.maybeStillDeleted();
		}
	}
}

void AuctionWriter::resetAuctionProcess() {
	auto it = auctionsState.begin();
	for(; it != auctionsState.end();) {
		AuctionInfo& auctionInfo = it->second;
		if(auctionInfo.processStatus == AuctionInfo::PS_NotProcessed) {
			log(LL_Error, "Post process auction: found auction in PS_NotProcessed state, should have been set to PS_Deleted, uid: 0x%08X\n", auctionInfo.uid);
			++it;
		} else if(auctionInfo.processStatus == AuctionInfo::PS_Deleted) {
			it = auctionsState.erase(it);
		} else {
			auctionInfo.resetProcess();
			++it;
		}
	}

	resetCategoryTime();
}

void AuctionWriter::serializeAuctionInfos(bool doFullDump, std::vector<uint8_t> &output)
{
	output.clear();

	AUCTION_FILE auctionFile;
	serializeHeader(auctionFile.header, doFullDump ? DT_Full : DT_Diff);

	auctionFile.auctions.reserve(auctionsState.size());

	auto it = auctionsState.begin();
	for(; it != auctionsState.end(); ++it) {
		const AuctionInfo& auctionInfo = getAuctionInfoFromValue(*it);
		DiffType diffType = doFullDump ? D_Base : auctionInfo.getAuctionDiffType();

		if(diffType >= D_Invalid)
			logStatic(LL_Error, getStaticClassName(), "Invalid diff flag: %d for auction 0x%08X\n", auctionInfo.processStatus, auctionInfo.uid);

		if((diffType == D_Unmodified) && !doFullDump)
			continue;

		AUCTION_INFO auctionItem;
		auctionInfo.serialize(&auctionItem, false);

		auctionFile.auctions.push_back(auctionItem);
	}

	MessageBuffer buffer(auctionFile.getSize(auctionFile.header.file_version), auctionFile.header.file_version);
	auctionFile.serialize(&buffer);
	if(buffer.checkFinalSize() == false) {
		log(LL_Error, "Wrong buffer size, size: %d, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
	} else {
		output.insert(output.end(), buffer.getData(), buffer.getData() + buffer.getSize());
	}
}

void AuctionWriter::importDump(AUCTION_FILE *auctionFile)
{
	deserializeHeader(auctionFile->header);

	auctionsState.clear();
	for(size_t i = 0; i < auctionFile->auctions.size(); i++) {
		AUCTION_INFO& auctionInfo = auctionFile->auctions[i];
		auctionsState.insert(std::make_pair(auctionInfo.uid, AuctionInfo::createFromDump(&auctionInfo)));
	}
}
