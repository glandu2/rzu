#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "P4ComputeStats.h"
#include "PipelineState.h"
#include <stdint.h>

template<class T> class DbQueryJob;

struct DB_InsertItem {
	static cval<std::string>& connectionString;
	struct Input {
		int32_t uid;
		DbDateTime created_time_min;
		DbDateTime created_time_max;
		int8_t original_duration_type;
		int16_t category;
		int64_t price;
		int64_t original_bid_price;
		std::string seller;

		uint32_t handle;
		int32_t code;
		int64_t item_uid;
		int64_t count;
		int32_t ethereal_durability;
		uint32_t endurance;
		uint8_t enhance;
		uint8_t level;
		uint16_t enhance_chance;
		uint32_t flag;
		int32_t socket[4];
		uint32_t awaken_option_value[5];
		int32_t awaken_option_data[5];
		uint32_t random_type[10];
		int64_t random_value_1[10];
		int64_t random_value_2[10];
		int32_t remain_time;
		uint8_t elemental_effect_type;
		int32_t elemental_effect_remain_time;
		int32_t elemental_effect_attack_point;
		int32_t elemental_effect_magic_point;
		int32_t appearance_code;
		uint32_t summon_code;
		uint32_t item_effect_id;
	};

	struct Output {};

	static bool addAuction(std::vector<DB_InsertItem::Input>& auctions, const AUCTION_INFO& auctionInfo);
	static bool createTable(DbConnectionPool* dbConnectionPool);

protected:
	static bool fillItemInfo(Input& input, uint32_t epic, const std::vector<uint8_t>& auctionInfo);
};

class P5InsertDataToSqlServer
    : public PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                          std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>> {
	DECLARE_CLASSNAME(P5InsertDataToSqlServer, 0)
public:
	P5InsertDataToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
	std::vector<DB_InsertItem::Input> dbInputs;
};
