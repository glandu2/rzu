#pragma once

#include "Database/DbQueryJob.h"
#include "GameTypes.h"

namespace GameServer {

class DB_Item {
public:
	game_sid_t sid;
	int32_t owner_id;
	int32_t summon_id;
	int32_t idx;
	int32_t code;
	int64_t count;
	int32_t level;
	int32_t enhance;
	int32_t ethereal_durability;
	int32_t endurance;
	int32_t flag;
	int32_t gcode;
	int32_t wear_info;
	int32_t socket[4];
	int32_t awaken_sid;
	int32_t remain_time;
	int8_t elemental_effect_type;
	int32_t elemental_effect_attack_point;
	int32_t elemental_effect_magic_point;
	int32_t appearance_code;
	SQL_TIMESTAMP_STRUCT elemental_effect_expire_time;
	SQL_TIMESTAMP_STRUCT create_time;
	SQL_TIMESTAMP_STRUCT update_time;
};

struct DB_ItemBinding {
	struct Input {
		game_sid_t owner_id;
	};

	typedef DB_Item Output;
};

}  // namespace GameServer

