#define NOMINMAX
#include "gtest/gtest.h"
#include "AuctionSimpleDiffWriter.h"
#include "AuctionFile.h"
#include "Core/Utils.h"
#include <random>

struct AuctionSimpleInputTestData {
	AuctionSimpleInputTestData() {
		uid = rand();
		time = rand();
		category = rand() % 18;
	}

	uint32_t uid;
	int64_t time;
	uint16_t diffType;
	uint16_t category;

	std::vector<uint8_t> rawData;
	std::vector<uint8_t> expectedData;

	void updateRawData() {
		if(rawData.size() > 0)
			return;

		rawData.resize(rand()%1024 + 1);
		for(size_t i = 0; i < rawData.size(); i++)
			rawData[i] = i & 0xFF;

		expectedData = rawData;
	}
};

struct AuctionSimpleOuputTestData : public AuctionSimpleInputTestData {
	AuctionSimpleOuputTestData()
	    : AuctionSimpleInputTestData()
	{}

	int64_t previousTime;
};

static void addAuction(AuctionSimpleDiffWriter* auctionWriter, AuctionSimpleInputTestData& testData)
{
	testData.updateRawData();
	auctionWriter->addAuctionInfo(AuctionUid(testData.uid), testData.time, testData.category, testData.rawData.data(), testData.rawData.size());
}

static void dumpAuctions(AuctionSimpleDiffWriter* auctionWriter, AUCTION_SIMPLE_FILE* auctionFile, bool doFulldump) {
	std::vector<uint8_t> auctionData;

	auctionWriter->dumpAuctions(auctionData, doFulldump);
	MessageBuffer messageBuffer(auctionData.data(), auctionData.size(), AUCTION_LATEST);
	auctionFile->deserialize(&messageBuffer);

	ASSERT_TRUE(messageBuffer.checkFinalSize());
}

static void expectAuction(AUCTION_SIMPLE_FILE* auctionFile,
                          AuctionSimpleOuputTestData& auctionOutput)
{
	bool auctionFound = false;

	for(size_t i = 0; i < auctionFile->auctions.size(); i++) {
		const AUCTION_SIMPLE_INFO& auction = auctionFile->auctions[i];

		if(auction.uid != auctionOutput.uid)
			continue;

		auctionFound = true;

		EXPECT_EQ(auctionOutput.uid, auction.uid);
		EXPECT_EQ(auctionOutput.time, auction.time);
		EXPECT_EQ(auctionOutput.previousTime, auction.previousTime);

		EXPECT_EQ(auctionOutput.diffType, auction.diffType);
		EXPECT_EQ(auctionOutput.category, auction.category);

		EXPECT_EQ(auctionOutput.expectedData.size(), auction.data.size());
		if(auctionOutput.expectedData.size() == auction.data.size())
			EXPECT_EQ(0, memcmp(auction.data.data(), auctionOutput.expectedData.data(), auction.data.size())) << "auction.data est différent de l'attendu";
	}

	EXPECT_TRUE(auctionFound) << "Auction " << auctionOutput.uid << " not found";
}

TEST(auction_simple_mode, no_auction) {
	AuctionSimpleDiffWriter auctionWriter(18);
	AuctionSimpleOuputTestData auctionData;
	AUCTION_SIMPLE_FILE auctionFile;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData.time - rand()%1000);
	auctionWriter.endCategory(auctionData.category, auctionData.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile, true);
	EXPECT_EQ(0, memcmp("RHS", auctionFile.header.signature, 4));
	EXPECT_EQ(AUCTION_LATEST, auctionFile.header.file_version);
	EXPECT_EQ(DT_Full, auctionFile.header.dumpType);
	EXPECT_EQ(18, auctionFile.header.categories.size());
	EXPECT_EQ(0, auctionFile.auctions.size());

	dumpAuctions(&auctionWriter, &auctionFile, false);
	EXPECT_EQ(0, memcmp("RHS", auctionFile.header.signature, 4));
	EXPECT_EQ(AUCTION_LATEST, auctionFile.header.file_version);
	EXPECT_EQ(DT_Diff, auctionFile.header.dumpType);
	EXPECT_EQ(18, auctionFile.header.categories.size());
	EXPECT_EQ(0, auctionFile.auctions.size());
}

TEST(auction_simple_mode, added_3_auction) {
	AuctionSimpleDiffWriter auctionWriter(18);
	AuctionSimpleOuputTestData auctionData;
	AuctionSimpleOuputTestData auctionData2;
	AuctionSimpleOuputTestData auctionData3;
	AUCTION_SIMPLE_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.previousTime = 0;

	auctionData2.uid = auctionData.uid + 5454; // be sure they are differents
	auctionData2.diffType = D_Added;
	auctionData2.previousTime = 0;

	auctionData3.uid = auctionData2.uid + 5454; // be sure they are differents
	auctionData3.diffType = D_Added;
	auctionData3.previousTime = 0;

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, std::min(std::min(auctionData3.time, auctionData2.time), auctionData.time) - rand()%1000);
	addAuction(&auctionWriter, auctionData);
	addAuction(&auctionWriter, auctionData2);
	addAuction(&auctionWriter, auctionData3);
	auctionWriter.endCategory(auctionData.category, std::max(std::max(auctionData3.time, auctionData2.time), auctionData.time) + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile, true);
	ASSERT_EQ(3, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);
	expectAuction(&auctionFile, auctionData2);
	expectAuction(&auctionFile, auctionData3);

	dumpAuctions(&auctionWriter, &auctionFile, false);
	ASSERT_EQ(3, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);
	expectAuction(&auctionFile, auctionData2);
	expectAuction(&auctionFile, auctionData3);
}

TEST(auction_simple_mode, added_update_unmodified_removed) {
	AuctionSimpleDiffWriter auctionWriter(18);
	AuctionSimpleOuputTestData auctionData;
	AuctionSimpleOuputTestData auctionData2;
	AuctionSimpleOuputTestData auctionData3;
	AuctionSimpleOuputTestData auctionData4;
	AuctionSimpleOuputTestData auctionData5;
	AuctionSimpleOuputTestData auctionData6;
	AUCTION_SIMPLE_FILE auctionFile;

	auctionData.diffType = D_Added;
	auctionData.previousTime = 0;
	auctionData.category = 5;
	auctionData.updateRawData();

	auctionData2 = auctionData;
	auctionData2.diffType = D_Updated;
	auctionData2.previousTime = auctionData2.time;
	auctionData2.time += 5000;
	auctionData2.rawData[auctionData2.rawData.size() - 1]++;
	auctionData2.expectedData = auctionData2.rawData;

	auctionData3 = auctionData2;
	auctionData3.diffType = D_Unmodified;
	auctionData3.previousTime = auctionData3.time;
	auctionData3.time += 5000;

	auctionData4 = auctionData3;
	auctionData4.diffType = D_MaybeDeleted;
	auctionData4.previousTime = auctionData4.time;
	auctionData4.time += 5000;

	auctionData5.diffType = D_Added;
	auctionData5.previousTime = auctionData3.time;
	auctionData5.time = auctionData5.previousTime + 1000 + rand()%1000;
	auctionData5.category = auctionData4.category - 1;
	auctionData5.updateRawData();

	auctionData6.diffType = D_Added;
	auctionData6.previousTime = auctionData4.time - rand()%1000;
	auctionData6.time = auctionData6.previousTime + 1000 + rand()%1000;
	auctionData6.category = auctionData4.category + 1;
	auctionData6.updateRawData();

	// Added auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData.time - rand()%1000);
	addAuction(&auctionWriter, auctionData);
	auctionWriter.endCategory(auctionData.category, auctionData.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);

	dumpAuctions(&auctionWriter, &auctionFile, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData);

	// Updated auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData2.time - rand()%1000);
	addAuction(&auctionWriter, auctionData2);
	auctionWriter.endCategory(auctionData.category, auctionData2.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData2);

	dumpAuctions(&auctionWriter, &auctionFile, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData2);

	// Unmodified auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData3.time - rand()%1000);
	addAuction(&auctionWriter, auctionData3);
	auctionWriter.endCategory(auctionData.category, auctionData3.time);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData3);

	dumpAuctions(&auctionWriter, &auctionFile, false);
	ASSERT_EQ(0, auctionFile.auctions.size());

	// Deleted auction
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData.category, auctionData6.previousTime);
	auctionWriter.endCategory(auctionData.category, auctionData4.time);
	auctionWriter.endProcess();

	auctionData4.diffType = D_Deleted;

	dumpAuctions(&auctionWriter, &auctionFile, true);
	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData4);

	dumpAuctions(&auctionWriter, &auctionFile, false);
	ASSERT_EQ(1, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData4);

	// Re-added auctions
	auctionWriter.beginProcess();
	auctionWriter.beginCategory(auctionData5.category, auctionData5.time - rand()%1000);
	addAuction(&auctionWriter, auctionData5);
	auctionWriter.endCategory(auctionData5.category, auctionData5.time + rand()%1000);
	auctionWriter.beginCategory(auctionData6.category, auctionData6.time - rand()%1000);
	addAuction(&auctionWriter, auctionData6);
	auctionWriter.endCategory(auctionData6.category, auctionData6.time + rand()%1000);
	auctionWriter.endProcess();

	dumpAuctions(&auctionWriter, &auctionFile, true);
	ASSERT_EQ(2, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData5);
	expectAuction(&auctionFile, auctionData6);

	dumpAuctions(&auctionWriter, &auctionFile, false);
	ASSERT_EQ(2, auctionFile.auctions.size());
	expectAuction(&auctionFile, auctionData5);
	expectAuction(&auctionFile, auctionData6);
}
