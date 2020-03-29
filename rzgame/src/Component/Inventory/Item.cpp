#include "Item.h"
#include "Config/GlobalConfig.h"
#include "Database/DB_Item.h"

namespace GameServer {

Item::Item(DB_Item* dbItem) {
	summon_id = ar_handle_t(dbItem->summon_id);
	idx = dbItem->idx;
	code = dbItem->code;
	count = dbItem->count;
	level = dbItem->level;
	enhance = dbItem->enhance;
	ethereal_durability = dbItem->ethereal_durability;
	endurance = dbItem->endurance;
	flag = dbItem->flag;
	gcode = dbItem->gcode;
	memcpy(socket, dbItem->socket, sizeof(socket));
	awaken_sid = dbItem->awaken_sid;
	remain_time = dbItem->remain_time;
	elemental_effect_type = dbItem->elemental_effect_type;
	elemental_effect_attack_point = dbItem->elemental_effect_attack_point;
	elemental_effect_magic_point = dbItem->elemental_effect_magic_point;
	appearance_code = dbItem->appearance_code;

	addObject(dbItem->sid, this);
}

void Item::fillInventoryItem(TS_ITEM_INFO& item) {
	item.own_summon_handle = summon_id;
	item.index = idx;
	item.base_info.code = code;
	item.base_info.count = count;
	item.base_info.flag = flag;
	item.base_info.handle = handle;
	item.base_info.ethereal_durability = ethereal_durability;
	item.base_info.endurance = endurance;
	item.base_info.enhance = enhance;
	item.base_info.level = level;
	item.base_info.uid = sid;
	item.base_info.remain_time = remain_time;
	memcpy(item.base_info.socket, socket, Utils_countOf(socket));
	item.base_info.elemental_effect_remain_time = 0;
	item.base_info.elemental_effect_attack_point = 0;
	item.base_info.elemental_effect_magic_point = 0;
	item.base_info.elemental_effect_type = 0;
	item.base_info.appearance_code = appearance_code;
	memset(item.base_info.awaken_option.data, 0, sizeof(item.base_info.awaken_option.data));
	memset(item.base_info.awaken_option.value, 0, sizeof(item.base_info.awaken_option.value));
}

}  // namespace GameServer
