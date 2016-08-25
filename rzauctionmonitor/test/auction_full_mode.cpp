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

TEST(auction_full_mode, added_auction) {
	AuctionWriter auctionWriter(18);
	AuctionOuputTestData auctionData(AuctionInfo::BF_NoBid, AuctionInfo::DT_Medium);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 24*3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 24*3600;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData.time - rand()%1000);
	addAuction(&auctionWriter, auctionData);
	auctionWriter.endCategory(auctionData.category, auctionData.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile);

	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);
}

TEST(auction_full_mode, added_2_auction) {
	AuctionWriter auctionWriter(18);
	AuctionOuputTestData auctionData(AuctionInfo::BF_NoBid, AuctionInfo::DT_Long);
	AuctionOuputTestData auctionData2(AuctionInfo::BF_NoBid, AuctionInfo::DT_Short);
	AUCTION_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.deleted = false;
	auctionData.deletedCount = 0;
	auctionData.previousTime = 0;
	auctionData.estimatedEndTimeFromAdded = true;
	auctionData.estimatedEndTimeMin = 72*3600;
	auctionData.estimatedEndTimeMax = auctionData.time + 72*3600;

	auctionData2.uid = auctionData.uid + 5454; // be sure they are differents
	auctionData2.diffType = D_Added;
	auctionData2.deleted = false;
	auctionData2.deletedCount = 0;
	auctionData2.previousTime = 0;
	auctionData2.estimatedEndTimeFromAdded = true;
	auctionData2.estimatedEndTimeMin = 6*3600;
	auctionData2.estimatedEndTimeMax = auctionData2.time + 6*3600;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, std::min(auctionData2.time, auctionData.time) - rand()%1000);
	addAuction(&auctionWriter, auctionData);
	addAuction(&auctionWriter, auctionData2);
	auctionWriter.endCategory(auctionData.category, std::max(auctionData2.time, auctionData.time) + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile);

	ASSERT_EQ(2, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);
	expectAuction(&auctionFile, auctionData2);
}
