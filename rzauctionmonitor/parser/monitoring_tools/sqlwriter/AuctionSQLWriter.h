#pragma once

#include "AuctionFile.h"
#include "Database/DbQueryJobRef.h"

struct DB_Item {
	static cval<std::string>& connectionString;
	struct Input {
		int32_t uid;
		int16_t diff_flag;
		DbDateTime previous_time;
		DbDateTime time;
		DbDateTime estimatedEndMin;
		DbDateTime estimatedEndMax;
		int16_t category;
		uint8_t duration_type;
		int64_t bid_price;
		int64_t price;
		std::string seller;
		int8_t bid_flag;

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
		float random_value_1[10];
		float random_value_2[10];
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

	static void addAuction(std::vector<DB_Item::Input>& auctions, const AUCTION_INFO& auctionInfo);
	static void addAuction(std::vector<DB_Item::Input>& auctions, const AUCTION_SIMPLE_INFO& auctionInfo);
	static bool createTable(DbConnectionPool* dbConnectionPool);

protected:
	static void fillItemInfo(Input& input, const std::vector<uint8_t>& auctionInfo);
};
