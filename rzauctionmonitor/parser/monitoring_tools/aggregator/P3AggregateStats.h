#ifndef P3AGGREGATESTATS_H
#define P3AGGREGATESTATS_H

#include "Core/BackgroundWork.h"
#include "Core/Object.h"
#include "IPipeline.h"
#include "P2ParseAuction.h"
#include <stdint.h>

struct AUCTION_FILE;
struct AUCTION_INFO;
struct TS_ITEM_BASE_INFO;

struct AuctionSummary {
	int32_t code;
	int16_t enhance;
	int64_t price;
	bool isSold;
	int64_t count;

	AuctionSummary() : price(0), isSold(false), count(0) {}
};

class P3AggregateStats
    : public PipelineStep<std::unique_ptr<AuctionDumpToAggregate>,
                          std::tuple<std::string, time_t, AUCTION_FILE, std::unordered_map<uint32_t, AuctionSummary>>,
                          char>,
      public Object {
	DECLARE_CLASSNAME(P3AggregateStats, 0)
public:
	P3AggregateStats();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	int processWork(std::shared_ptr<WorkItem> item);
	void afterWork(std::shared_ptr<WorkItem> item, int status);

	bool skipItem(const AUCTION_INFO& auctionInfo, const TS_ITEM_BASE_INFO& item);

private:
	enum Category {
		C_Weapon = 0,
		C_Armor = 1,
		C_Shield = 2,
		C_Helmet = 3,
		C_Gloves = 4,
		C_Boots = 5,
		C_Belt = 6,
		C_Cloak = 7,
		C_Accessory = 8,
		C_DecorativeItem = 9,
		C_SkillCard = 10,
		C_SummonerCard = 11,
		C_Bag = 12,
		C_StrikeCube = 13,
		C_DefenseCube = 14,
		C_SkillCube = 15,
		C_SoulStone = 16,
		C_CraftMaterials = 17,
		C_Supplies = 18
	};

private:
	BackgroundWork<P3AggregateStats, std::shared_ptr<WorkItem>> work;

	std::unordered_map<uint32_t, AuctionSummary> auctionsByUid;
};

#endif
