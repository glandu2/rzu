#include "AuctionComplexDiffWriter.h"
#include "Core/PrintfFormats.h"
#include <algorithm>
#include <time.h>

AuctionComplexDiffWriter::AuctionComplexDiffWriter(size_t categoryCount)
    : AuctionGenericWriter(categoryCount), diffMode(false) {}

void AuctionComplexDiffWriter::addAuctionInfo(const AUCTION_SIMPLE_INFO* auction) {
	uint32_t epic;

	if(auction->category >= categoryTimeManager.getCategoryNumber()) {
		log(LL_Error,
		    "Auction diff with invalid category: uid: 0x%08X, diffType: %d, category: %d, category number: %d\n",
		    auction->uid,
		    auction->diffType,
		    auction->category,
		    (int) categoryTimeManager.getCategoryNumber());
		return;
	}

	categoryTimeManager.adjustCategoryTimeRange(auction->category, auction->time);

	DiffType diffType = (DiffType)(auction->diffType);
	uint64_t estimatedTruePreviousTime = 0;
	AuctionComplexData* auctionInfo = getAuction(AuctionUid(auction->uid));

	if(auction->previousTime) {
		estimatedTruePreviousTime = auction->previousTime;
	} else if(auctionInfo && (time_t) auctionInfo->getTimeMin() <=
	                             categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category)) {
		// auction previous time before category begin time, don't trust it and set previous time
		estimatedTruePreviousTime = categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category);
	}

	epic = parseEpic(auction->epic, auction->time);

	switch(diffType) {
		case D_Base:
		case D_Added:
		case D_Updated: {
			if(auctionInfo) {
				if(diffMode && diffType == D_Added && !auctionInfo->isDeleted()) {
					log(LL_Error,
					    "Added auction already exists: 0x%08X, with state: %d\n",
					    auction->uid,
					    auctionInfo->getProcessStatus());
				}

				if(estimatedTruePreviousTime) {
					auctionInfo->setTimeMin(estimatedTruePreviousTime);
				}

				auctionInfo->update(auction->time, epic, auction->data.data(), auction->data.size());
			} else {
				if(diffType == D_Updated) {
					log(LL_Error, "Updated auction not found: 0x%08X\n", auction->uid);
				} else if(auction->previousTime &&
				          auction->previousTime !=
				              categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category)) {
					log(LL_Debug,
					    "Added auction previous time mismatch: category previous begin time: %" PRId64
					    ", new auction previous time: %" PRId64 "\n",
					    (int64_t) categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category),
					    (int64_t) auction->previousTime);
				}

				uint64_t previousTimeToUse =
				    auction->previousTime
				        ? auction->previousTime
				        : categoryTimeManager.getEstimatedPreviousCategoryBeginTime(auction->category);
				addAuction(new AuctionComplexData(auction->uid,
				                                  previousTimeToUse,
				                                  auction->time,
				                                  auction->category,
				                                  epic,
				                                  auction->data.data(),
				                                  auction->data.size()));
			}
			break;
		}
		case D_MaybeDeleted:
			break;
		case D_Deleted: {
			if(!auctionInfo) {
				log(LL_Error, "Deleted auction not found: 0x%08X\n", auction->uid);
			} else {
				if(auctionInfo->getProcessStatus() != AuctionComplexData::PS_NotProcessed) {
					log(LL_Error,
					    "Deleted auction is not in PS_NotProcessed flag: uid: 0x%08X, flag: %d\n",
					    auction->uid,
					    auctionInfo->getProcessStatus());
				}
				/*
				                if(categoryTimeManager.getCategoryTime(auction->category).end == 0) {
				                    log(LL_Info, "Category %d end time is 0, setting to deletion time %" PRIu64 "\n",
				   auction->category, auction->time); categoryTimeManager.getCategoryTime(auction->category).end =
				   auction->time; } else if(categoryTimeManager.getCategoryTime(auction->category).end != auction->time)
				   { log(LL_Warning, "Deleted auction time is not category end time: uid: 0x%08X, time: %" PRIu64 ",
				   category: %d, category end time: %" PRIu64 "\n", auction->uid, auction->time, auction->category,
				   categoryTimeManager.getCategoryTime(auction->category).end);
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

void AuctionComplexDiffWriter::beginProcess() {
	AuctionGenericWriter::beginProcess([](AuctionComplexData* data) { data->beginProcess(); });
}

void AuctionComplexDiffWriter::endProcess() {
	struct CategoryTimeInfo {
		uint64_t begin;
		uint64_t end;
	};

	// Cache category times to have faster lookups (getEstimatedXXX compute each time the result)
	std::vector<CategoryTimeInfo> categoryTimeInfo;
	categoryTimeInfo.resize(categoryTimeManager.getCategoryNumber());
	for(size_t i = 0; i < categoryTimeInfo.size(); i++) {
		categoryTimeInfo[i].begin = categoryTimeManager.getEstimatedCategoryBeginTime(i);
		categoryTimeInfo[i].end = categoryTimeManager.getEstimatedCategoryEndTime(i);
	}

	AuctionGenericWriter::endProcess([this, &categoryTimeInfo](AuctionComplexData* data) {
		data->endProcess(
		    categoryTimeInfo[data->getCategory()].begin, categoryTimeInfo[data->getCategory()].end, diffMode);
	});
}

void AuctionComplexDiffWriter::setDiffInputMode(bool diffMode) {
	this->diffMode = diffMode;
}

void AuctionComplexDiffWriter::dumpAuctions(std::vector<uint8_t>& output, bool doFullDump, bool alwaysWithData) {
	AUCTION_FILE file = exportDump(doFullDump, alwaysWithData);
	serialize(file, output);
}

AUCTION_FILE AuctionComplexDiffWriter::exportDump(bool doFullDump, bool alwaysWithData) {
	AUCTION_FILE auctionFile;

	categoryTimeManager.serializeHeader(auctionFile.header, doFullDump ? DT_Full : DT_Diff);

	size_t auctionCount = getAuctionCount();
	log(LL_Info, "Exporting %d auctions\n", (int) auctionCount);
	auctionFile.auctions.reserve(auctionCount);

	forEachAuction([&auctionFile, doFullDump, alwaysWithData](AuctionComplexData* auctionInfo) {
		if(!doFullDump && !auctionInfo->outputInPartialDump())
			return;

		auctionFile.auctions.emplace_back();
		AUCTION_INFO& auctionItem = auctionFile.auctions.back();
		auctionInfo->serialize(&auctionItem, alwaysWithData);
	});

	return auctionFile;
}

void AuctionComplexDiffWriter::importDump(const AUCTION_FILE* auctionFile) {
	clearAuctions();
	categoryTimeManager.deserializeHeader(auctionFile->header);
	for(size_t i = 0; i < auctionFile->auctions.size(); i++) {
		const auto& auctionInfo = auctionFile->auctions[i];
		addAuction(AuctionComplexData::createFromDump(&auctionInfo));
	}
	log(LL_Info, "Imported %d auctions\n", (int) getAuctionCount());
}

uint32_t AuctionComplexDiffWriter::parseEpic(uint32_t inputEpic, int64_t timestamp) {
	if(inputEpic != 0xFFFFFF) {
		if(inputEpic == EPIC_9_6_7 && timestamp >= 1634716800)  // 2021-10-20T08:00:00Z
			return EPIC_9_6_8;
		if(inputEpic == EPIC_9_7_2 && timestamp >= 1674115200)  // 2023-01-19T08:00:00Z
			return EPIC_9_8_0;
		return inputEpic;
	}

	if(timestamp < 1552986000)  // 2019-03-19T09:00:00.000Z
		return EPIC_9_5_3;
	else
		return EPIC_9_6_1;
}
