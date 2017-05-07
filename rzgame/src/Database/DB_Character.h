#ifndef DBCHARACTER_H
#define DBCHARACTER_H

#include "GameTypes.h"
#include "Database/DbQueryJob.h"

namespace GameServer {

class DB_Character {

public:
	std::string name;
	char account[61];
	game_sid_t party_id;
	int32_t permission;
	float x;
	float y;
	float z;
	int8_t layer;
	int32_t race;
	int32_t sex;
	int32_t lv;
	int64_t exp;
	int32_t hp;
	int32_t mp;
	int32_t stamina;
	int32_t job;
	int32_t job_depth;
	int32_t jlv;
	int64_t jp;
	int64_t total_jp;
	int32_t talent_point;
	int32_t jobs[3];
	int32_t jlvs[3];
	int32_t immoral_point;
	int32_t pkc;
	int32_t dkc;
	int32_t huntaholic_point;
	int32_t huntaholic_enter_count;
	int32_t ethereal_stone_durability;
	int64_t gold;
	int32_t chaos;
	int32_t skin_color;
	int32_t model[5];
	int32_t hair_color_index;
	int32_t hair_color_rgb;
	int32_t hide_equip_flag;
	int32_t face_texture_id;
	game_sid_t belt[6];
	game_sid_t summon_id[6];
	game_sid_t main_summon;
	game_sid_t sub_summon;
	int32_t remain_summon_time;
	game_sid_t pet;
	game_sid_t main_title;
	game_sid_t sub_title[5];
	int32_t remain_title_time;
	int32_t arena_point;
	SQL_TIMESTAMP_STRUCT arena_block_time;
	int32_t arena_penalty_count;
	SQL_TIMESTAMP_STRUCT arena_penalty_dec_time;
	int32_t arena_mvp_count;
	int32_t arena_record[3][2];
	std::string alias;
	int32_t chat_block_time;
	int32_t adv_chat_count;
	int32_t name_changed;
	int32_t auto_used;
	bool pkmode;
	int32_t otp_value;
	SQL_TIMESTAMP_STRUCT otp_date;
	std::string flag_list;
	std::string client_info;
};

// binding with character table
struct DB_CharacterBinding {
	struct Input {
		game_sid_t sid;
	};

	typedef DB_Character Output;
};

}

#endif // DBCHARACTER_H
