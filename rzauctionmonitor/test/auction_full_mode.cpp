#include "AuctionComplexDiffWriter.h"
#include "AuctionFile.h"
#include "Core/Utils.h"
#include "gtest/gtest.h"
#include <random>

struct AuctionInputTestData {
	AuctionInputTestData(BidFlag bidFlag, DurationType durationType) {
		uid = rand();
		time = rand();
		bid_flag = bidFlag;
		bid_price = rand();
		category = rand() % 18;
		duration_type = durationType;
		price = rand();
		epic = rand();
		if(epic == 0xFFFFFF)  // avoid special 'not set' value
			epic = 0;
		seller = std::to_string(rand());
	}

	uint32_t uid;
	int64_t time;
	int64_t previousTime;
	uint16_t diffType;
	uint16_t category;
	DurationType duration_type;
	int64_t bid_price;
	int64_t price;
	uint32_t epic;
	std::string seller;
	BidFlag bid_flag;

	std::vector<uint8_t> rawData;
	std::vector<uint8_t> expectedData;

	void updateRawData() {
		if(rawData.size() > 0)
			return;
		AuctionDataEnd auctionData;
		memset(&auctionData, 0, sizeof(auctionData));

		auctionData.bid_flag = (int8_t) bid_flag;
		auctionData.bid_price = bid_price;
		auctionData.duration_type = (int8_t) duration_type;
		auctionData.price = price;
		strcpy(auctionData.seller, seller.c_str());

		rawData.resize(rand() % 1024 + sizeof(auctionData));
		for(size_t i = 0; i < rawData.size(); i++)
			rawData[i] = i & 0xFF;

		memcpy(&rawData[rawData.size() - sizeof(auctionData)], &auctionData, sizeof(auctionData));

		expectedData = rawData;
	}
};

struct AuctionOuputTestData : public AuctionInputTestData {
	AuctionOuputTestData(BidFlag bidFlag, DurationType durationType) : AuctionInputTestData(bidFlag, durationType) {}

	bool estimatedEndTimeFromAdded;
	int64_t estimatedEndTimeMin;
	int64_t estimatedEndTimeMax;
	bool deleted;
	uint8_t deletedCount;
};

namespace AuctionComplexNs {
static void addAuction(AuctionComplexDiffWriter* auctionWriter, AuctionInputTestData& testData) {
	testData.updateRawData();
	AUCTION_SIMPLE_INFO auctionInfo;

	auctionInfo.uid = testData.uid;
	auctionInfo.time = testData.time;
	auctionInfo.previousTime = testData.previousTime;
	auctionInfo.diffType = D_Base;
	auctionInfo.category = testData.category;
	auctionInfo.epic = testData.epic;
	auctionInfo.data = testData.rawData;

	auctionWriter->addAuctionInfo(&auctionInfo);
}

static void addAuctionDiff(AuctionComplexDiffWriter* auctionWriter, AuctionOuputTestData& testData) {
	testData.updateRawData();
	AUCTION_SIMPLE_INFO auctionInfo;

	auctionInfo.uid = testData.uid;
	auctionInfo.time = testData.time;
	auctionInfo.previousTime = testData.previousTime;
	auctionInfo.diffType = testData.diffType;
	auctionInfo.category = testData.category;
	auctionInfo.epic = testData.epic;
	auctionInfo.data = testData.rawData;

	auctionWriter->addAuctionInfo(&auctionInfo);
}

static void dumpAuctions(AuctionComplexDiffWriter* auctionWriter,
                         AUCTION_FILE* auctionFile,
                         bool doFullDump,
                         bool alwaysWithData) {
	std::vector<uint8_t> auctionData;

	auctionWriter->dumpAuctions(auctionData, doFullDump, alwaysWithData);
	MessageBuffer messageBuffer(auctionData.data(), auctionData.size(), AUCTION_LATEST);
	auctionFile->deserialize(&messageBuffer);

	ASSERT_TRUE(messageBuffer.checkFinalSize());
}

static void expectAuction(AUCTION_FILE* auctionFile, AuctionOuputTestData& auctionOutput, bool expectData) {
	bool auctionFound = false;

	for(size_t i = 0; i < auctionFile->auctions.size(); i++) {
		const AUCTION_INFO& auction = auctionFile->auctions[i];

		if(auction.uid != auctionOutput.uid)
			continue;

		auctionFound = true;

		EXPECT_EQ(auctionOutput.uid, auction.uid);
		EXPECT_EQ(auctionOutput.time, auction.time);
		EXPECT_EQ(auctionOutput.previousTime, auction.previousTime);
		EXPECT_EQ(auctionOutput.estimatedEndTimeFromAdded, auction.estimatedEndTimeFromAdded);
		EXPECT_EQ(auctionOutput.estimatedEndTimeMin, auction.estimatedEndTimeMin);
		EXPECT_EQ(auctionOutput.estimatedEndTimeMax, auction.estimatedEndTimeMax);

		EXPECT_EQ(auctionOutput.diffType, auction.diffType);
		EXPECT_EQ(auctionOutput.bid_flag, auction.bid_flag);
		EXPECT_EQ(auctionOutput.bid_price, auction.bid_price);
		EXPECT_EQ(auctionOutput.category, auction.category);
		EXPECT_EQ(auctionOutput.deleted, auction.deleted);
		EXPECT_EQ(auctionOutput.deletedCount, auction.deletedCount);
		EXPECT_EQ(auctionOutput.duration_type, auction.duration_type);
		EXPECT_EQ(auctionOutput.price, auction.price);
		EXPECT_STREQ(auctionOutput.seller.c_str(), auction.seller.c_str());
		EXPECT_EQ(auctionOutput.epic, auction.epic);

		if(expectData) {
			EXPECT_EQ(auctionOutput.expectedData.size(), auction.data.size());
			if(auctionOutput.expectedData.size() == auction.data.size()) {
				EXPECT_EQ(0, memcmp(auction.data.data(), auctionOutput.expectedData.data(), auction.data.size()))
				    << "auction.data est différent de l'attendu";
			}
		} else {
			EXPECT_EQ(0, auction.data.size());
		}
	}

	EXPECT_TRUE(auctionFound) << "Auction " << auctionOutput.uid << " not found";
}

static void reimportAuctions(std::unique_ptr<AuctionComplexDiffWriter>& auctionWriter) {
	AUCTION_FILE auctionFile;
	bool doImport = false;

	if(auctionWriter) {
		AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, true, true);
		doImport = true;
	}
	auctionWriter.reset(new AuctionComplexDiffWriter(18));
	auctionWriter->setDiffInputMode(true);
	if(doImport) {
		auctionWriter->importDump(&auctionFile);
	}
}
}  // namespace AuctionComplexNs

TEST(auction_full_mode, no_auction) {
	AuctionComplexDiffWriter auctionWriter(18);
	AuctionOuputTestData auctionData(BF_NoBid, DT_Medium);
	AUCTION_FILE auctionFile;

	time_t categoryBegin = auctionData.time - rand() % 1000;
	time_t categoryEnd = auctionData.time + rand() % 1000;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, categoryBegin);
	auctionWriter.endCategory(auctionData.category, categoryEnd);
	auctionWriter.endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);

	EXPECT_EQ(0, memcmp("RAH", auctionFile.header.signature, 4));
	EXPECT_EQ(AUCTION_V6, auctionFile.header.file_version);
	EXPECT_EQ(DT_Full, auctionFile.header.dumpType);
	EXPECT_EQ(18, auctionFile.header.categories.size());
	EXPECT_EQ(categoryBegin, auctionFile.header.categories[auctionData.category].beginTime);
	EXPECT_EQ(categoryEnd, auctionFile.header.categories[auctionData.category].endTime);
	EXPECT_EQ(0, auctionFile.auctions.size());

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, true);

	EXPECT_EQ(0, memcmp("RAH", auctionFile.header.signature, 4));
	EXPECT_EQ(AUCTION_LATEST, auctionFile.header.file_version);
	EXPECT_EQ(DT_Diff, auctionFile.header.dumpType);
	EXPECT_EQ(18, auctionFile.header.categories.size());
	EXPECT_EQ(categoryBegin, auctionFile.header.categories[auctionData.category].beginTime);
	EXPECT_EQ(categoryEnd, auctionFile.header.categories[auctionData.category].endTime);
	EXPECT_EQ(0, auctionFile.auctions.size());
}

TEST(auction_full_mode, added_3_auction) {
	AuctionComplexDiffWriter auctionWriter(18);
	AuctionOuputTestData auctionData(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData2(BF_MyBid, DT_Long);
	AuctionOuputTestData auctionData3(BF_Bidded, DT_Short);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 24 * 3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 24 * 3600;

	auctionData2.uid = auctionData.uid + 5454;  // be sure they are differents
	auctionData2.diffType = D_Added;
	auctionData2.deleted = false;
	auctionData2.deletedCount = 0;
	auctionData2.previousTime = 0;
	auctionData2.estimatedEndTimeFromAdded = true;
	auctionData2.estimatedEndTimeMin = 72 * 3600;
	auctionData2.estimatedEndTimeMax = auctionData2.time + 72 * 3600;

	auctionData3.uid = auctionData2.uid + 5454;  // be sure they are differents
	auctionData3.diffType = D_Added;
	auctionData3.deleted = false;
	auctionData3.deletedCount = 0;
	auctionData3.previousTime = 0;
	auctionData3.estimatedEndTimeFromAdded = true;
	auctionData3.estimatedEndTimeMin = 6 * 3600;
	auctionData3.estimatedEndTimeMax = auctionData3.time + 6 * 3600;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(
	    auctionData.category,
	    std::min(std::min(auctionData3.time, auctionData2.time), auctionData.time) - rand() % 1000);
	AuctionComplexNs::addAuction(&auctionWriter, auctionData);
	AuctionComplexNs::addAuction(&auctionWriter, auctionData2);
	AuctionComplexNs::addAuction(&auctionWriter, auctionData3);
	auctionWriter.endCategory(
	    auctionData.category,
	    std::max(std::max(auctionData3.time, auctionData2.time), auctionData.time) + rand() % 1000);
	auctionWriter.endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);

	ASSERT_EQ(3, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, true);
	AuctionComplexNs::expectAuction(&auctionFile, auctionData3, true);

	// diff dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);

	ASSERT_EQ(3, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, true);
	AuctionComplexNs::expectAuction(&auctionFile, auctionData3, true);
}

TEST(auction_full_mode, added_update_unmodified_removed) {
	AuctionComplexDiffWriter auctionWriter(18);
	AuctionOuputTestData auctionData(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData2(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData3(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData4(BF_NoBid, DT_Medium);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 24 * 3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 24 * 3600;

	auctionData2 = auctionData;
	auctionData2.diffType = D_Updated;
	auctionData2.bid_flag = BF_MyBid;
	auctionData2.previousTime = auctionData2.time;
	auctionData2.time += 5000;

	auctionData3 = auctionData2;
	auctionData3.diffType = D_Unmodified;
	auctionData3.previousTime = auctionData3.time;
	auctionData3.time += 5000;

	auctionData4 = auctionData3;
	auctionData4.diffType = D_MaybeDeleted;
	auctionData4.deleted = true;
	auctionData4.deletedCount = 1;
	auctionData4.previousTime = auctionData4.time;
	auctionData4.time += 5000;

	// Added auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData.time - rand() % 1000);
	AuctionComplexNs::addAuction(&auctionWriter, auctionData);
	auctionWriter.endCategory(auctionData.category, auctionData.time + rand() % 1000);
	auctionWriter.endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);

	// Updated auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData2.time - rand() % 1000);
	AuctionComplexNs::addAuction(&auctionWriter, auctionData2);
	auctionWriter.endCategory(auctionData.category, auctionData2.time + rand() % 1000);
	auctionWriter.endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, false);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, true);

	// Unmodified auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData3.time - rand() % 1000);
	AuctionComplexNs::addAuction(&auctionWriter, auctionData3);
	auctionWriter.endCategory(auctionData.category, auctionData3.time + rand() % 1000);
	auctionWriter.endProcess();

	// full dump without data
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData3, false);

	// full dump with data
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData3, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, true);
	ASSERT_EQ(0, auctionFile.auctions.size());

	// MaybeDeleted auction
	// Copy updated data from actionData3 as auctionData4 is never added
	auctionData4.rawData = auctionData3.rawData;
	auctionData4.expectedData = auctionData3.expectedData;

	int i;
	for(i = 0; i < 3; i++) {
		auctionWriter.beginProcess();
		auctionWriter.beginCategory(auctionData.category, auctionData4.time + 5000 * i - 1000);
		auctionWriter.endCategory(auctionData.category, auctionData4.time + 5000 * i);
		auctionWriter.endProcess();

		auctionData4.deletedCount = i + 1;

		// full dump
		AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);
		ASSERT_EQ(1, auctionFile.auctions.size());
		AuctionComplexNs::expectAuction(&auctionFile, auctionData4, false);

		// partial dump
		AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, true);
		ASSERT_EQ(0, auctionFile.auctions.size());
	}

	// Deleted auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData4.time + 5000 * i - 1000);
	auctionWriter.endCategory(auctionData.category, auctionData4.time + 5000 * i);
	auctionWriter.endProcess();

	auctionData4.diffType = D_Deleted;
	auctionData4.deletedCount = 4;

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, false);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, true);
}

TEST(auction_full_mode, added_update_unmodified_removed_diff) {
	AuctionComplexDiffWriter auctionWriter(18);
	AuctionOuputTestData auctionData(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData2(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData3(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData4(BF_NoBid, DT_Medium);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 24 * 3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 24 * 3600;

	auctionData2 = auctionData;
	auctionData2.diffType = D_Updated;
	auctionData2.bid_flag = BF_MyBid;
	auctionData2.previousTime = auctionData2.time;
	auctionData2.time += 5000;

	auctionData3 = auctionData2;
	auctionData3.diffType = D_Unmodified;
	auctionData3.previousTime = auctionData2.time;
	auctionData3.time = auctionData2.time;

	auctionData4 = auctionData3;
	auctionData4.diffType = D_Deleted;
	auctionData4.deleted = true;
	auctionData4.deletedCount = 1;
	auctionData4.previousTime = auctionData2.time;
	auctionData4.time = auctionData2.time + 5000;

	// Set diff mode
	auctionWriter.setDiffInputMode(true);

	// Added auction
	auctionWriter.beginProcess();
	AuctionComplexNs::addAuctionDiff(&auctionWriter, auctionData);
	auctionWriter.endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);

	// Updated auction
	auctionWriter.beginProcess();
	AuctionComplexNs::addAuctionDiff(&auctionWriter, auctionData2);
	auctionWriter.endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, false);

	// Unmodified auction
	auctionData3.rawData = auctionData2.rawData;
	auctionData3.expectedData = auctionData2.expectedData;

	auctionWriter.beginProcess();
	auctionWriter.endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData3, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);
	ASSERT_EQ(0, auctionFile.auctions.size());

	// Deleted auction (MaybeDeleted true type)
	auctionData4.rawData = auctionData3.rawData;
	auctionData4.expectedData = auctionData3.expectedData;

	auctionWriter.beginProcess();
	AuctionComplexNs::addAuctionDiff(&auctionWriter, auctionData4);
	auctionWriter.endProcess();

	auctionData4.diffType = D_MaybeDeleted;

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);
	ASSERT_EQ(0, auctionFile.auctions.size());

	for(int i = 1; i < 3; i++) {
		// MaybeDeleted auction
		auctionWriter.beginProcess();
		auctionWriter.endProcess();

		auctionData4.deletedCount = i + 1;

		// full dump
		AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
		ASSERT_EQ(1, auctionFile.auctions.size());
		AuctionComplexNs::expectAuction(&auctionFile, auctionData4, true);

		// partial dump
		AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);
		ASSERT_EQ(0, auctionFile.auctions.size());
	}

	// Deleted auction
	auctionWriter.beginProcess();
	auctionWriter.endProcess();

	auctionData4.diffType = D_Deleted;
	auctionData4.deletedCount = 4;

	// full dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, false);
}

TEST(auction_full_mode, import_state) {
	std::unique_ptr<AuctionComplexDiffWriter> auctionWriter;
	AuctionOuputTestData auctionData(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData2(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData3(BF_NoBid, DT_Medium);
	AuctionOuputTestData auctionData4(BF_NoBid, DT_Medium);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 24 * 3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 24 * 3600;

	auctionData2 = auctionData;
	auctionData2.diffType = D_Updated;
	auctionData2.bid_flag = BF_MyBid;
	auctionData2.previousTime = auctionData2.time;
	auctionData2.time += 5000;

	auctionData3 = auctionData2;
	auctionData3.diffType = D_Unmodified;
	auctionData3.previousTime = auctionData2.time;
	auctionData3.time = auctionData2.time;

	auctionData4 = auctionData3;
	auctionData4.diffType = D_Deleted;
	auctionData4.deleted = true;
	auctionData4.deletedCount = 1;
	auctionData4.previousTime = auctionData2.time;
	auctionData4.time = auctionData2.time + 5000;

	// Added auction
	AuctionComplexNs::reimportAuctions(auctionWriter);
	auctionWriter->beginProcess();
	AuctionComplexNs::addAuctionDiff(auctionWriter.get(), auctionData);
	auctionWriter->endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, false, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);

	// Updated auction
	AuctionComplexNs::reimportAuctions(auctionWriter);
	auctionWriter->beginProcess();
	AuctionComplexNs::addAuctionDiff(auctionWriter.get(), auctionData2);
	auctionWriter->endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, false, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, false);

	// Unmodified auction
	auctionData3.rawData = auctionData2.rawData;
	auctionData3.expectedData = auctionData2.expectedData;

	AuctionComplexNs::reimportAuctions(auctionWriter);
	auctionWriter->beginProcess();
	auctionWriter->endProcess();

	// full dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData3, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, false, false);
	ASSERT_EQ(0, auctionFile.auctions.size());

	// Deleted auction (MaybeDeleted true type)
	auctionData4.rawData = auctionData3.rawData;
	auctionData4.expectedData = auctionData3.expectedData;

	AuctionComplexNs::reimportAuctions(auctionWriter);
	auctionWriter->beginProcess();
	AuctionComplexNs::addAuctionDiff(auctionWriter.get(), auctionData4);
	auctionWriter->endProcess();

	auctionData4.diffType = D_MaybeDeleted;

	// full dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, false, false);
	ASSERT_EQ(0, auctionFile.auctions.size());

	for(int i = 1; i < 3; i++) {
		// MaybeDeleted auction
		AuctionComplexNs::reimportAuctions(auctionWriter);
		auctionWriter->beginProcess();
		auctionWriter->endProcess();

		auctionData4.deletedCount = i + 1;

		// full dump
		AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, true, true);
		ASSERT_EQ(1, auctionFile.auctions.size());
		AuctionComplexNs::expectAuction(&auctionFile, auctionData4, true);

		// partial dump
		AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, false, false);
		ASSERT_EQ(0, auctionFile.auctions.size());
	}

	// Deleted auction
	AuctionComplexNs::reimportAuctions(auctionWriter);
	auctionWriter->beginProcess();
	auctionWriter->endProcess();

	auctionData4.diffType = D_Deleted;
	auctionData4.deletedCount = 4;

	// full dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, true, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, true);

	// partial dump
	AuctionComplexNs::dumpAuctions(auctionWriter.get(), &auctionFile, false, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData4, false);
}

TEST(auction_full_mode, estimated_time_adjust) {
	AuctionComplexDiffWriter auctionWriter(18);
	AuctionOuputTestData auctionData(BF_NoBid, DT_Long);
	AuctionOuputTestData auctionData2(BF_NoBid, DT_Long);
	AuctionOuputTestData auctionData3(BF_NoBid, DT_Long);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 72 * 3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 72 * 3600;

	auctionData2 = auctionData;
	auctionData2.diffType = D_Updated;
	auctionData2.duration_type = DT_Medium;
	auctionData2.previousTime = auctionData.time + 72 * 3600 - 24 * 3600 - 1000;
	auctionData2.time = auctionData.time + 72 * 3600 - 24 * 3600 + 500;
	auctionData2.estimatedEndTimeFromAdded = false;
	auctionData2.estimatedEndTimeMin = auctionData.time + 72 * 3600 - 1000;
	auctionData2.estimatedEndTimeMax = auctionData.time + 72 * 3600;

	auctionData3 = auctionData2;
	auctionData3.diffType = D_Updated;
	auctionData3.duration_type = DT_Short;
	auctionData3.previousTime = auctionData.time + 72 * 3600 - 6 * 3600 - 1500;
	auctionData3.time = auctionData.time + 72 * 3600 - 6 * 3600 - 500;
	auctionData3.estimatedEndTimeMin = auctionData.time + 72 * 3600 - 1000;
	auctionData3.estimatedEndTimeMax = auctionData.time + 72 * 3600 - 500;

	// Set diff mode
	auctionWriter.setDiffInputMode(true);

	// Added auction
	auctionWriter.beginProcess();
	AuctionComplexNs::addAuctionDiff(&auctionWriter, auctionData);
	auctionWriter.endProcess();

	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, false);

	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);

	// Updated auction
	auctionWriter.beginProcess();
	AuctionComplexNs::addAuctionDiff(&auctionWriter, auctionData2);
	auctionWriter.endProcess();

	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData2, true);

	// Updated auction
	auctionWriter.beginProcess();
	AuctionComplexNs::addAuctionDiff(&auctionWriter, auctionData3);
	auctionWriter.endProcess();

	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, false, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	AuctionComplexNs::expectAuction(&auctionFile, auctionData3, false);
}

TEST(auction_full_mode, write_read_auctions_file_empty_gz) {
	AuctionComplexDiffWriter auctionWriter(19);
	std::vector<uint8_t> fileData;
	AUCTION_FILE auctionFile;

	auctionWriter.dumpAuctions(fileData, true, true);
	auctionWriter.writeAuctionDataToFile("test_auctions", "auction_file_empty.bin.gz", fileData);

	fileData.clear();
	EXPECT_TRUE(auctionWriter.readAuctionDataFromFile("test_auctions", "auction_file_empty.bin.gz", fileData));

	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	ASSERT_EQ(0, auctionFile.auctions.size());
}

static void testWriteReadAuctionComplexFile(std::string name) {
	struct Header {
		char sign[4];
		uint32_t version;
	};

	static const size_t AUCTIONS_COUNT = 200;

	AuctionComplexDiffWriter auctionWriter(18);
	AuctionOuputTestData auctionData(BF_NoBid, DT_Medium);
	AUCTION_FILE auctionFile;
	AUCTION_FILE auctionFile2;
	std::vector<uint8_t> fileDataBefore;
	std::vector<uint8_t> fileDataAfter;
	std::vector<uint8_t> fileDataAfter2;

	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;

	auctionWriter.beginProcess();
	for(size_t i = 0; i < AUCTIONS_COUNT; i++) {
		auctionData.uid = i;  // be sure they are differents
		auctionData.diffType = D_Added;
		auctionData.previousTime = i * 2;
		auctionData.estimatedEndTimeMin = 24 * 3600 + auctionData.previousTime;
		auctionData.estimatedEndTimeMax = auctionData.time + 24 * 3600;
		AuctionComplexNs::addAuction(&auctionWriter, auctionData);
	}
	auctionWriter.endProcess();

	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile, true, true);
	auctionWriter.dumpAuctions(fileDataBefore, true, true);
	ASSERT_EQ(AUCTIONS_COUNT, auctionFile.auctions.size());

	for(size_t i = 0; i < AUCTIONS_COUNT; i++) {
		auctionData.uid = i;  // be sure they are differents
		auctionData.diffType = D_Added;
		auctionData.previousTime = i * 2;
		auctionData.estimatedEndTimeMin = 24 * 3600 + auctionData.previousTime;
		auctionData.estimatedEndTimeMax = auctionData.time + 24 * 3600;
		AuctionComplexNs::expectAuction(&auctionFile, auctionData, true);
	}

	// dump to file
	auctionWriter.dumpAuctions(fileDataAfter, true, true);
	auctionWriter.writeAuctionDataToFile("test_auctions", name, fileDataAfter);

	EXPECT_TRUE(auctionWriter.readAuctionDataFromFile("test_auctions", name, fileDataAfter2));

	EXPECT_EQ(fileDataBefore.size(), fileDataAfter2.size());
	if(fileDataBefore.size() == fileDataAfter2.size()) {
		EXPECT_EQ(0, memcmp(fileDataBefore.data(), fileDataAfter2.data(), fileDataBefore.size()));
	}

	uint32_t version = ((Header*) fileDataAfter2.data())->version;
	MessageBuffer structBuffer(fileDataAfter2.data(), fileDataAfter2.size(), version);

	auctionFile.deserialize(&structBuffer);
	ASSERT_TRUE(structBuffer.checkFinalSize());

	EXPECT_EQ(DT_Full, auctionFile.header.dumpType);

	auctionWriter.importDump(&auctionFile);

	// check all auctions
	AuctionComplexNs::dumpAuctions(&auctionWriter, &auctionFile2, true, true);
	ASSERT_EQ(AUCTIONS_COUNT, auctionFile2.auctions.size());

	for(size_t i = 0; i < AUCTIONS_COUNT; i++) {
		auctionData.uid = i;  // be sure they are differents
		auctionData.diffType = D_Added;
		auctionData.previousTime = i * 2;
		auctionData.estimatedEndTimeMin = 24 * 3600 + auctionData.previousTime;
		auctionData.estimatedEndTimeMax = auctionData.time + 24 * 3600;
		AuctionComplexNs::expectAuction(&auctionFile2, auctionData, true);
	}
}

TEST(auction_full_mode, write_read_auctions_file_big_gz) {
	testWriteReadAuctionComplexFile("auction_file_big.bin.gz");
}

TEST(auction_full_mode, write_read_auctions_file_big_bin) {
	testWriteReadAuctionComplexFile("auction_file_big.bin");
}
