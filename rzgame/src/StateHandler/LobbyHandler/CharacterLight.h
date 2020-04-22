#pragma once

#include "Database/DbQueryJob.h"
#include "GameTypes.h"

namespace GameServer {

class CharacterLight {
public:
	game_sid_t sid;
	std::string name;
	uint32_t race;
	uint32_t sex;
	uint32_t lv;
	uint32_t jlv;
	uint64_t exp;
	uint32_t hp;
	uint32_t mp;
	uint32_t job;
	uint32_t permission;
	uint32_t skin_color;
	uint32_t model[5];
	uint32_t hair_color_index;
	uint32_t hair_color_rgb;
	uint32_t hide_equip_flag;
	uint32_t texture_id;
	char create_time[30];
	char login_time[30];
	uint32_t login_count;
	char logout_time[30];
	uint32_t play_time;
};

struct CharacterLightBinding {
	struct Input {
		uint32_t account_id;
	};

	typedef CharacterLight Output;
};

}  // namespace GameServer

