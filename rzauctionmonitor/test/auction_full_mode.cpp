#include "gtest/gtest.h"
#include "AuctionWriter.h"
#include "AuctionFile.h"

TEST(auction_full_mode, added_auction) {
	AuctionWriter auctionWriter(18);
	AuctionDataEnd auctionData;

	// parameters
	size_t category = 5;
	uint32_t uid = 1;
	uint64_t time = 2;

	auctionData.bid_flag = (int8_t) AuctionInfo::BF_NoBid;
	auctionData.bid_price = 42;
	auctionData.duration_type = (int8_t) AuctionInfo::DT_Medium;
	auctionData.price = 142;
	strcpy(auctionData.seller, "Jean");

	auctionWriter.beginProcess();
	auctionWriter.beginCategory(category, time);
	auctionWriter.addAuctionInfo(uid, time+1, category, (const uint8_t*) &auctionData, sizeof(auctionData));
	auctionWriter.endCategory(category, time+2);
	auctionWriter.endProcess();

	const std::unordered_map<uint32_t, AuctionInfo>& auctions = auctionWriter.getAuctions();

	ASSERT_EQ(1, auctions.size());
	ASSERT_EQ(1, auctions.count(1));

	const AuctionInfo& auction = auctions.at(1);

	EXPECT_EQ(D_Added, auction.getAuctionDiffType());
	EXPECT_EQ(AuctionInfo::BF_NoBid, auction.getBidFlag());
	EXPECT_EQ(42, auction.getBidPrice());
	EXPECT_EQ(category, auction.getCategory());
	EXPECT_EQ(false, auction.getDeleted());
	EXPECT_EQ(0, auction.getDeletedCount());
	EXPECT_EQ(AuctionInfo::DT_Medium, auction.getDurationType());
	EXPECT_EQ(time+1, auction.getUpdateTime());
	EXPECT_EQ(0, auction.getPreviousUpdateTime());
	EXPECT_EQ(time+1 + 24*3600, auction.getEstimatedEndTimeMax());
	EXPECT_EQ(24*3600, auction.getEstimatedEndTimeMin());
	EXPECT_EQ(142, auction.getPrice());
	EXPECT_EQ(AuctionInfo::PS_Added, auction.getProcessStatus());
	EXPECT_STREQ("Jean", auction.getSeller().c_str());
	EXPECT_EQ(uid, auction.getUid());

	auto rawData = auction.getRawData();
	ASSERT_EQ(sizeof(auctionData), rawData.size());
	EXPECT_EQ(0, memcmp(&auctionData, rawData.data(), sizeof(auctionData)));
}
