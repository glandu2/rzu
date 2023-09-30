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
	item.code = code;
	item.count = count;
	item.flag = flag;
	item.handle = handle;
	item.ethereal_durability = ethereal_durability;
	item.endurance = endurance;
	item.enhance = enhance;
	item.level = level;
	item.uid = sid;
	item.remain_time = remain_time;
	memcpy(item.sockets.socket, socket, Utils_countOf(socket));
	item.elemental_effect.remain_time = 0;
	item.elemental_effect.attack_point = 0;
	item.elemental_effect.magic_point = 0;
	item.elemental_effect.type = 0;
	item.appearance_code = appearance_code;
	memset(item.awaken_option.data, 0, sizeof(item.awaken_option.data));
	memset(item.awaken_option.value, 0, sizeof(item.awaken_option.value));
}

}  // namespace GameServer
