#include "AuctionSimpleDiffWriter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include <time.h>
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include <algorithm>
#include "Packet/MessageBuffer.h"

AuctionSimpleDiffWriter::AuctionSimpleDiffWriter(size_t categoryCount) : AuctionCommonWriter(categoryCount)
{
}

void AuctionSimpleDiffWriter::addAuctionInfo(uint32_t uid, uint64_t time, uint16_t category, const uint8_t* data, size_t len)
{
	auto it = auctionsState.find(uid);
	if(it != auctionsState.end()) {
		AuctionSimpleData& auctionInfo = it->second;

		if(auctionInfo.update(time, data, len))
			log(LL_Debug, "Auction info modified: 0x%08X\n", uid);
	} else {
		auctionsState.insert(std::make_pair(uid,
		                                    AuctionSimpleData(uid,
		                                                getEstimatedPreviousCategoryBeginTime(category),
		                                                time,
														category,
		                                                data, len)));
		log(LL_Debug, "Auction info added: %d\n", uid);
	}

	adjustCategoryTimeRange(category, time);
}

void AuctionSimpleDiffWriter::processRemainingAuctions() {
	auto it = auctionsState.begin();
	for(; it != auctionsState.end(); ++it) {
		AuctionSimpleData& auctionInfo = it->second;
		if(auctionInfo.processStatus == AuctionSimpleData::PS_NotProcessed) {
			time_t endCategory = getCategoryTime(auctionInfo.category).end;
			if(endCategory == 0)
				log(LL_Warning, "End of category %d is time 0\n", auctionInfo.category);

			auctionInfo.remove(endCategory);
			log(LL_Debug, "Auction info removed: 0x%08X\n", auctionInfo.uid);
		}
	}
}

void AuctionSimpleDiffWriter::resetAuctionProcess() {
	auto it = auctionsState.begin();
	for(; it != auctionsState.end();) {
		AuctionSimpleData& auctionInfo = it->second;
		if(auctionInfo.processStatus == AuctionSimpleData::PS_NotProcessed) {
			log(LL_Error, "Post process auction: found auction in PS_NotProcessed state, should have been set to PS_Deleted, uid: 0x%08X\n", auctionInfo.uid);
			++it;
		} else if(auctionInfo.processStatus == AuctionSimpleData::PS_Deleted) {
			it = auctionsState.erase(it);
		} else {
			auctionInfo.resetProcess();
			++it;
		}
	}

	resetCategoryTime();
}

void AuctionSimpleDiffWriter::serializeAuctionInfos(bool doFullDump, std::vector<uint8_t> &output)
{
	output.clear();

	AUCTION_SIMPLE_FILE auctionFile;
	serializeHeader(auctionFile.header, doFullDump ? DT_Full : DT_Diff);

	auctionFile.auctions.reserve(auctionsState.size());

	auto it = auctionsState.begin();
	for(; it != auctionsState.end(); ++it) {
		const AuctionSimpleData& auctionInfo = getAuctionInfoFromValue(*it);
		DiffType diffType = doFullDump ? D_Base : auctionInfo.getAuctionDiffType();

		if(diffType >= D_Invalid)
			logStatic(LL_Error, getStaticClassName(), "Invalid diff flag: %d for auction 0x%08X\n", auctionInfo.processStatus, auctionInfo.uid);

		if((diffType == D_Unmodified) && !doFullDump)
			continue;

		AUCTION_SIMPLE_INFO auctionItem;
		auctionInfo.serialize(&auctionItem);

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

void AuctionSimpleDiffWriter::importDump(AUCTION_SIMPLE_FILE *auctionFile)
{
	deserializeHeader(auctionFile->header);

	auctionsState.clear();
	for(size_t i = 0; i < auctionFile->auctions.size(); i++) {
		AUCTION_SIMPLE_INFO& auctionInfo = auctionFile->auctions[i];
		auctionsState.insert(std::make_pair(auctionInfo.uid, AuctionSimpleData::createFromDump(&auctionInfo)));
	}
}
