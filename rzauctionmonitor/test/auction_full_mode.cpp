#define NOMINMAX
#include "gtest/gtest.h"
#include "AuctionWriter.h"
#include "AuctionFile.h"
#include "Core/Utils.h"
#include <random>

struct AuctionInputTestData {
	AuctionInputTestData(AuctionInfo::BidFlag bidFlag, AuctionInfo::DurationType durationType) {
		uid = rand();
		time = rand();
		bid_flag = bidFlag;
		bid_price = rand();
		category = rand() % 18;
		duration_type = durationType;
		price = rand();
	}

	uint32_t uid;
	int64_t time;
	uint16_t diffType;
	uint16_t category;
	AuctionInfo::DurationType duration_type;
	int64_t bid_price;
	int64_t price;
	std::string seller;
	AuctionInfo::BidFlag bid_flag;

	std::vector<uint8_t> rawData;
};

struct AuctionOuputTestData : public AuctionInputTestData {
	AuctionOuputTestData(AuctionInfo::BidFlag bidFlag, AuctionInfo::DurationType durationType)
	    : AuctionInputTestData(bidFlag, durationType)
	{}

	int64_t previousTime;
	bool estimatedEndTimeFromAdded;
	int64_t estimatedEndTimeMin;
	int64_t estimatedEndTimeMax;
	bool deleted;
	uint8_t deletedCount;
};

static void addAuction(AuctionWriter* auctionWriter, AuctionInputTestData& testData)
{
	AuctionDataEnd auctionData;

	auctionData.bid_flag = (int8_t) testData.bid_flag;
	auctionData.bid_price = testData.bid_price;
	auctionData.duration_type = (int8_t) testData.duration_type;
	auctionData.price = testData.price;
	strcpy(auctionData.seller, testData.seller.c_str());

	testData.rawData.resize(rand()%1024 + sizeof(auctionData));
	for(size_t i = 0; i < testData.rawData.size(); i++)
		testData.rawData[i] = rand() & 0xFF;

	memcpy(&testData.rawData[testData.rawData.size()-sizeof(auctionData)], &auctionData, sizeof(auctionData));

	auctionWriter->addAuctionInfo(testData.uid, testData.time, testData.category, testData.rawData.data(), testData.rawData.size());
}

static void dumpAuctions(AuctionWriter* auctionWriter, AUCTION_FILE* auctionFile) {
	std::vector<uint8_t> auctionData;

	auctionWriter->dumpAuctions(auctionData, true);
	MessageBuffer messageBuffer(auctionData.data(), auctionData.size(), AUCTION_LATEST);
	auctionFile->deserialize(&messageBuffer);

	ASSERT_TRUE(messageBuffer.checkFinalSize());
}

static void expectAuction(AUCTION_FILE* auctionFile,
                          AuctionOuputTestData& auctionOutput)
{
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

		EXPECT_EQ(auctionOutput.rawData.size(), auction.data.size());
		if(auctionOutput.rawData.size() == auction.data.size())
			EXPECT_EQ(0, memcmp(auction.data.data(), auctionOutput.rawData.data(), auction.data.size())) << "auction.data est différent de l'attendu";
	}

	EXPECT_TRUE(auctionFound) << "Auction " << auctionOutput.uid << " not found";
}

TEST(auction_full_mode, no_auction) {
	AuctionWriter auctionWriter(18);
	AuctionOuputTestData auctionData(AuctionInfo::BF_NoBid, AuctionInfo::DT_Medium);
	AUCTION_FILE auctionFile;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData.time - rand()%1000);
	auctionWriter.endCategory(auctionData.category, auctionData.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile);

	EXPECT_EQ(0, memcmp("RAH", auctionFile.signature, 4));
	EXPECT_EQ(AUCTION_LATEST, auctionFile.file_version);
	EXPECT_EQ(DT_Full, auctionFile.dumpType);
	EXPECT_EQ(18, auctionFile.categories.size());
	EXPECT_EQ(0, auctionFile.auctions.size());
}

TEST(auction_full_mode, added_3_auction) {
	AuctionWriter auctionWriter(18);
	AuctionOuputTestData auctionData(AuctionInfo::BF_NoBid, AuctionInfo::DT_Medium);
	AuctionOuputTestData auctionData2(AuctionInfo::BF_MyBid, AuctionInfo::DT_Long);
	AuctionOuputTestData auctionData3(AuctionInfo::BF_Bidded, AuctionInfo::DT_Short);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 24*3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 24*3600;

	auctionData2.uid = auctionData.uid + 5454; // be sure they are differents
	auctionData2.diffType = D_Added;
	auctionData2.deleted = false;
	auctionData2.deletedCount = 0;
	auctionData2.previousTime = 0;
	auctionData2.estimatedEndTimeFromAdded = true;
	auctionData2.estimatedEndTimeMin = 72*3600;
	auctionData2.estimatedEndTimeMax = auctionData2.time + 72*3600;

	auctionData3.uid = auctionData2.uid + 5454; // be sure they are differents
	auctionData3.diffType = D_Added;
	auctionData3.deleted = false;
	auctionData3.deletedCount = 0;
	auctionData3.previousTime = 0;
	auctionData3.estimatedEndTimeFromAdded = true;
	auctionData3.estimatedEndTimeMin = 6*3600;
	auctionData3.estimatedEndTimeMax = auctionData3.time + 6*3600;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, std::min(std::min(auctionData3.time, auctionData2.time), auctionData.time) - rand()%1000);
	addAuction(&auctionWriter, auctionData);
	addAuction(&auctionWriter, auctionData2);
	addAuction(&auctionWriter, auctionData3);
	auctionWriter.endCategory(auctionData.category, std::max(std::max(auctionData3.time, auctionData2.time), auctionData.time) + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile);

	ASSERT_EQ(3, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);
	expectAuction(&auctionFile, auctionData2);
	expectAuction(&auctionFile, auctionData3);
}

TEST(auction_full_mode, added_update_removed) {
	AuctionWriter auctionWriter(18);
	AuctionOuputTestData auctionData(AuctionInfo::BF_NoBid, AuctionInfo::DT_Medium);
	AuctionOuputTestData auctionData2(AuctionInfo::BF_MyBid, AuctionInfo::DT_Long);
	AuctionOuputTestData auctionData3(AuctionInfo::BF_Bidded, AuctionInfo::DT_Short);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 24*3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 24*3600;

	auctionData2 = auctionData;
	auctionData2.diffType = D_Updated;
	auctionData2.bid_flag = AuctionInfo::BF_MyBid;
	auctionData2.previousTime = auctionData2.time;
	auctionData2.time += 5000;

	auctionData3 = auctionData2;
	auctionData3.diffType = D_MaybeDeleted;
	auctionData3.deleted = true;
	auctionData3.deletedCount = 1;
	auctionData3.previousTime = auctionData3.time;
	auctionData3.time = auctionData2.time + 5000;

	Object::logStatic(Object::LL_Info, "added_update_removed", "Added auction\n");
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData.time - rand()%1000);
	addAuction(&auctionWriter, auctionData);
	auctionWriter.endCategory(auctionData.category, auctionData.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile);

	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);

	// Updated auction
	Object::logStatic(Object::LL_Info, "added_update_removed", "Updated auction\n");
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData2.time - rand()%1000);
	addAuction(&auctionWriter, auctionData2);
	auctionWriter.endCategory(auctionData.category, auctionData2.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile);

	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData2);


	auctionData3.rawData = auctionData2.rawData;

	for(int i = 0; i < 3; i++) {
		// MaybeDeleted auction
		Object::logStatic(Object::LL_Info, "added_update_removed", "Deleted auction\n");
		auctionWriter.beginProcess();
		auctionWriter.beginCategory(auctionData.category, auctionData2.time + 4000*(i+1));
		auctionWriter.endCategory(auctionData.category, auctionData2.time + 5000*(i+1));
		auctionWriter.endProcess();

		auctionData3.deletedCount = i+1;

		dumpAuctions(&auctionWriter, &auctionFile);

		ASSERT_EQ(1, auctionFile.auctions.size());
		expectAuction(&auctionFile, auctionData3);
	}

	// Deleted auction
	Object::logStatic(Object::LL_Info, "added_update_removed", "Deleted auction\n");
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData2.time + 4000);
	auctionWriter.endCategory(auctionData.category, auctionData2.time + 5000);
	auctionWriter.endProcess();

	auctionData3.diffType = D_Deleted;
	auctionData3.deletedCount = 4;

	dumpAuctions(&auctionWriter, &auctionFile);

	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData3);
}
