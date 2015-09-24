#ifndef CHARACTER_H
#define CHARACTER_H

#include "CharacterLight.h"

class CharacterDetails {
public:
	int32_t party_id;
	int32_t slot;
	int32_t permission;
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t layer;
	int32_t max_reached_level;
	int64_t last_decreased_exp;
	int32_t stamina;
	int32_t havoc;
	int8_t job_depth;
	int32_t current_jlv;
	int64_t jp;
	int64_t total_jp;
	int32_t talent_point;
	int32_t job[3];
	int32_t jlv[3];
	float immoral_point;
	int32_t cha;
	int32_t pkc;
	int32_t dkc;
	int32_t huntaholic_point;
	int32_t huntaholic_enter_count;
	int32_t ethereal_stone_durability;
	int64_t gold;
	int32_t chaos;
	int64_t belt[6];
	int32_t summon[6];
	int32_t main_summon;
	int32_t sub_summon;
	int32_t remain_summon_time;
	int32_t pet;
	int32_t main_title;
	int32_t sub_title[5];
	int32_t remain_title_time;
	int32_t arena_point;
	SQL_TIMESTAMP_STRUCT arena_block_time;
	int32_t arena_penalty_count;
	SQL_TIMESTAMP_STRUCT arena_penalty_dec_time;
	int32_t arena_mvp_count;
	int32_t arena_record[3][2];
	char alias[61];
	int32_t chat_block_time;
	int32_t adv_chat_count;
	int32_t name_changed;
	int32_t auto_used;
	int8_t pkmode;
	int32_t otp_value;
	SQL_TIMESTAMP_STRUCT otp_date;
	std::string flag_list;
	std::string client_info;
};

// binding with character table
struct CharacterDetailsBinding {
	struct Input {
		uint32_t sid;
	};

	typedef CharacterDetails Output;
};

#endif // CHARACTER_H
