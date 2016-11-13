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

void AuctionSimpleDiffWriter::dumpAuctions(std::vector<uint8_t> &output, bool doFullDump)
{
	AUCTION_SIMPLE_FILE file = exportDump(doFullDump);
	serialize(file, output);
}

AUCTION_SIMPLE_FILE AuctionSimpleDiffWriter::exportDump(bool doFullDump)
{
	AUCTION_SIMPLE_FILE auctionFile;

	categoryTimeManager.serializeHeader(auctionFile.header, doFullDump ? DT_Full : DT_Diff);
	strcpy(auctionFile.header.signature, "RHS");

	auctionFile.auctions.reserve(getAuctionCount());
	forEachAuction([&auctionFile, doFullDump](AuctionSimpleData* auctionInfo) {
		if(!doFullDump && !auctionInfo->outputInPartialDump())
			return;

		AUCTION_SIMPLE_INFO auctionItem;
		auctionInfo->serialize(&auctionItem);

		auctionFile.auctions.push_back(auctionItem);
	});

	return auctionFile;
}

void AuctionSimpleDiffWriter::importDump(const AUCTION_SIMPLE_FILE *auctionFile)
{
	clearAuctions();
	categoryTimeManager.deserializeHeader(auctionFile->header);
	for(size_t i = 0; i < auctionFile->auctions.size(); i++) {
		const auto& auctionInfo = auctionFile->auctions[i];
		addAuction(AuctionSimpleData::createFromDump(&auctionInfo));
	}
}
