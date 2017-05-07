#ifndef ITEMRESOURCE_H
#define ITEMRESOURCE_H

#include "RefDataLoader.h"
#include "Database/DbQueryJobRef.h"
#include <stdint.h>

namespace GameServer {

struct ItemResource {
	int32_t id;
	int32_t name_id;
	int32_t type;
	int32_t group;
	int32_t classType;
	int32_t wear_type;
	int32_t set_id;
	int32_t set_part_flag;
	int8_t grade;
	int32_t rank;
	int32_t level;
	int32_t enhance;
	int32_t socket;
	int32_t status_flag;
	bool limit_deva;
	bool limit_asura;
	bool limit_gaia;
	int8_t job_depth;
	bool limit_fighter;
	bool limit_hunter;
	bool limit_magician;
	bool limit_summoner;
	int32_t use_min_level;
	int32_t use_max_level;
	int32_t target_min_level;
	int32_t target_max_level;
	float range;
	float weight;
	float price;
	int32_t huntaholic_point;
	int32_t arena_point;
	int32_t ethereal_durability;
	int32_t endurance;
	int32_t material;
	int32_t summon_id;
	int32_t item_use_flag;
	int32_t available_period;
	int8_t decrease_type;
	float throw_range;
	int16_t base_type[4];
	float base_var1[4];
	float base_var2[4];
	int16_t opt_type[4];
	float opt_var1[4];
	float opt_var2[4];
	int32_t effect_id;
	int32_t enhance_id;
	int32_t skill_id;
	int32_t state_id;
	int32_t state_level;
	int32_t state_time;
	int32_t cool_time;
	int16_t cool_time_group;
	std::string script_text;
};

class ItemResourceBinding : public RefDataLoaderHelper<ItemResource, ItemResourceBinding> {
	DECLARE_CLASS(GameServer::ItemResourceBinding)
};

}

#endif // ITEMRESOURCE_H
