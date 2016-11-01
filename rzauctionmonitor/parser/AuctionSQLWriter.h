#ifndef AUCTIONSQLWRITER_H
#define AUCTIONSQLWRITER_H

#include "Database/DbQueryJobRef.h"
#include "AuctionFile.h"


#pragma pack(push, 1)
struct ItemData {
	uint32_t uid;
	uint32_t handle;
	int32_t code;
	int64_t item_uid;
	int64_t count;
	int32_t ethereal_durability;
	uint32_t endurance;
	uint8_t enhance;
	uint8_t level;
	int32_t flag;
	int32_t socket[4];
	uint32_t awaken_option_value[5];
	int32_t awaken_option_data[5];
	int32_t remain_time;
	uint8_t elemental_effect_type;
	int32_t elemental_effect_remain_time;
	int32_t elemental_effect_attack_point;
	int32_t elemental_effect_magic_point;
	int32_t appearance_code;
	int32_t unknown1[51];
	int16_t unknown2;
};
#pragma pack(pop)

struct DB_Item
{
	struct Input {
		int32_t uid;
		int16_t diff_flag;
		DbDateTime previous_time;
		DbDateTime time;
		DbDateTime estimatedEndMin;
		DbDateTime estimatedEndMax;
		int16_t category;
		int8_t duration_type;
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
		int32_t flag;
		int32_t socket[4];
		uint32_t awaken_option_value[5];
		int32_t awaken_option_data[5];
		int32_t remain_time;
		uint8_t elemental_effect_type;
		int32_t elemental_effect_remain_time;
		int32_t elemental_effect_attack_point;
		int32_t elemental_effect_magic_point;
		int32_t appearance_code;
	};

	struct Output {};

	static void addAuction(std::vector<DB_Item::Input>& auctions, const AUCTION_INFO& auctionInfo) {
		DB_Item::Input input;

		input.uid = auctionInfo.uid;
		input.diff_flag = auctionInfo.diffType;
		input.previous_time.setUnixTime(auctionInfo.previousTime);
		input.time.setUnixTime(auctionInfo.time);
		input.estimatedEndMin.setUnixTime(auctionInfo.estimatedEndTimeMin);
		input.estimatedEndMax.setUnixTime(auctionInfo.estimatedEndTimeMax);
		input.category = auctionInfo.category;

		ItemData* item = (ItemData*) auctionInfo.data.data();

		input.duration_type = auctionInfo.duration_type;
		input.bid_price = auctionInfo.bid_price;
		input.price = auctionInfo.price;
		input.seller = auctionInfo.seller;
		input.bid_flag = auctionInfo.bid_flag;

		input.handle = item->handle;
		input.code = item->code;
		input.item_uid = item->item_uid;
		input.count = item->count;
		input.ethereal_durability = item->ethereal_durability;
		input.endurance = item->endurance;
		input.enhance = item->enhance;
		input.level = item->level;
		input.flag = item->flag;
		memcpy(input.socket, item->socket, sizeof(input.socket));
		memcpy(input.awaken_option_value, item->awaken_option_value, sizeof(input.awaken_option_value));
		memcpy(input.awaken_option_data, item->awaken_option_data, sizeof(input.awaken_option_data));
		input.remain_time = item->remain_time;
		input.elemental_effect_type = item->elemental_effect_type;
		input.elemental_effect_remain_time = item->elemental_effect_remain_time;
		input.elemental_effect_attack_point = item->elemental_effect_attack_point;
		input.elemental_effect_magic_point = item->elemental_effect_magic_point;
		input.appearance_code = item->appearance_code;

		auctions.push_back(input);
	}
};

/*
CREATE TABLE [auctions](
	[uid] [int] NOT NULL,
	[diff_flag] [smallint] NOT NULL,
	[previous_time] [datetime] NOT NULL,
	[time] [datetime] NOT NULL,
	[estimated_end_min] [datetime] NOT NULL,
	[estimated_end_max] [datetime] NOT NULL,
	[category] [smallint] NOT NULL,
	[duration_type] [smallint] NOT NULL,
	[bid_price] [bigint] NOT NULL,
	[price] [bigint] NOT NULL,
	[seller] [varchar](32) NOT NULL,
	[bid_flag] [smallint] NOT NULL,
	[handle] [int] NOT NULL,
	[code] [int] NOT NULL,
	[item_uid] [bigint] NOT NULL,
	[count] [bigint] NOT NULL,
	[ethereal_durability] [int] NOT NULL,
	[endurance] [int] NOT NULL,
	[enhance] [smallint] NOT NULL,
	[level] [smallint] NOT NULL,
	[flag] [int] NOT NULL,
	[socket_0] [int] NOT NULL,
	[socket_1] [int] NOT NULL,
	[socket_2] [int] NOT NULL,
	[socket_3] [int] NOT NULL,
	[awaken_option_value_0] [int] NOT NULL,
	[awaken_option_value_1] [int] NOT NULL,
	[awaken_option_value_2] [int] NOT NULL,
	[awaken_option_value_3] [int] NOT NULL,
	[awaken_option_value_4] [int] NOT NULL,
	[awaken_option_data_0] [int] NOT NULL,
	[awaken_option_data_1] [int] NOT NULL,
	[awaken_option_data_2] [int] NOT NULL,
	[awaken_option_data_3] [int] NOT NULL,
	[awaken_option_data_4] [int] NOT NULL,
	[remain_time] [int] NOT NULL,
	[elemental_effect_type] [smallint] NOT NULL,
	[elemental_effect_remain_time] [int] NOT NULL,
	[elemental_effect_attack_point] [int] NOT NULL,
	[elemental_effect_magic_point] [int] NOT NULL,
	[appearance_code] [int] NOT NULL,
	PRIMARY KEY (
		[uid] ASC,
		[time] ASC
	)
);

*/

#endif
