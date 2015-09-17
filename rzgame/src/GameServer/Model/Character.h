#ifndef CHARACTER_H
#define CHARACTER_H

#include "CharacterLight.h"

class Character : public CharacterLight {
public:
	int32_t sid;
	char name[61];
	char account[60];
	int32_t account_id;
	int32_t party_id;
	int32_t slot;
	int32_t permission;
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t layer;
	int32_t race;
	int32_t sex;
	int32_t lv;
	int32_t max_reached_level;
	int64_t exp;
	int64_t last_decreased_exp;
	int32_t hp;
	int32_t mp;
	int32_t stamina;
	int32_t havoc;
	int32_t job;
	int8_t job_depth;
	int32_t jlv;
	int64_t jp;
	int64_t total_jp;
	int32_t talent_point;
	int32_t job_0;
	int32_t job_1;
	int32_t job_2;
	int32_t jlv_0;
	int32_t jlv_1;
	int32_t jlv_2;
	float immoral_point;
	int32_t cha;
	int32_t pkc;
	int32_t dkc;
	int32_t huntaholic_point;
	int32_t huntaholic_enter_count;
	int32_t ethereal_stone_durability;
	int64_t gold;
	int32_t chaos;
	int32_t skin_color;
	int32_t model_00;
	int32_t model_01;
	int32_t model_02;
	int32_t model_03;
	int32_t model_04;
	int32_t hair_color_index;
	int32_t hair_color_rgb;
	int32_t hide_equip_flag;
	int32_t texture_id;
	int64_t belt_00;
	int64_t belt_01;
	int64_t belt_02;
	int64_t belt_03;
	int64_t belt_04;
	int64_t belt_05;
	int32_t summon_0;
	int32_t summon_1;
	int32_t summon_2;
	int32_t summon_3;
	int32_t summon_4;
	int32_t summon_5;
	int32_t main_summon;
	int32_t sub_summon;
	int32_t remain_summon_time;
	int32_t pet;
	int32_t main_title;
	int32_t sub_title_0;
	int32_t sub_title_1;
	int32_t sub_title_2;
	int32_t sub_title_3;
	int32_t sub_title_4;
	int32_t remain_title_time;
	int32_t arena_point;
	SQL_TIMESTAMP_STRUCT arena_block_time;
	int32_t arena_penalty_count;
	SQL_TIMESTAMP_STRUCT arena_penalty_dec_time;
	int32_t arena_mvp_count;
	int32_t arena_record_0_0;
	int32_t arena_record_0_1;
	int32_t arena_record_1_0;
	int32_t arena_record_1_1;
	int32_t arena_record_2_0;
	int32_t arena_record_2_1;
	char alias[61];
	SQL_TIMESTAMP_STRUCT create_time;
	SQL_TIMESTAMP_STRUCT delete_time;
	SQL_TIMESTAMP_STRUCT login_time;
	int32_t login_count;
	SQL_TIMESTAMP_STRUCT logout_time;
	int32_t play_time;
	int32_t chat_block_time;
	int32_t adv_chat_count;
	int32_t name_changed;
	int32_t auto_used;
	int8_t pkmode;
	int32_t otp_value;
	SQL_TIMESTAMP_STRUCT otp_date;
	char flag_list[1000];
	char client_info[4096];
};

// binding with character table
struct PlayerBinding {
	struct Input {

	};

	struct Output : public Character {};
};

#endif // CHARACTER_H
