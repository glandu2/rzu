#ifndef AUTHSERVER_CREATE_CHARACTER_H
#define AUTHSERVER_CREATE_CHARACTER_H

#include "Database/DbQueryJob.h"
#include <stdint.h>

namespace GameServer {

struct CreateCharacterBinding {
	struct Input {
		uint64_t out_character_sid;

		std::string name;
		char account_name[61];
		uint32_t account_id;
		uint32_t slot;
		uint32_t x;
		uint32_t y;
		uint32_t z;
		uint32_t layer;
		uint32_t race;
		uint32_t sex;
		uint32_t level;
		uint32_t max_reached_level;
		uint32_t hp;
		uint32_t mp;
		uint32_t jlv;
		uint64_t jp;
		uint32_t cha;
		uint32_t skin_color;
		uint32_t model[5];
		uint32_t hair_color_index;
		uint32_t texture_id;
		uint64_t default_weapon_sid;
		uint32_t default_weapon_code;
		uint64_t default_armor_sid;
		uint32_t default_armor_code;
		uint64_t default_bag_sid;
		uint32_t default_bag_code;

		Input() : out_character_sid(0) {}
	};

	struct Output {};
};

}

#endif // AUTHSERVER_CREATE_CHARACTER_H
