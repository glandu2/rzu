#ifndef ITEM_H
#define ITEM_H

#include "../GameTypes.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "ModelObject.h"
#include "Core/Timer.h"

namespace GameServer {

class DB_Item;

class Item : public ModelObject<Item, 0x0> {
public:
	Item(DB_Item* dbItem);
	void fillInventoryItem(TS_ITEM_INFO& item);

public:
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
	int32_t socket[4];
	int32_t awaken_sid;
	int32_t remain_time;
	int8_t elemental_effect_type;
	int32_t elemental_effect_attack_point;
	int32_t elemental_effect_magic_point;
	int32_t appearance_code;

	TimerStatic elemental_effect_timer;
	TimerStatic expire_timer;
};

}

#endif // ITEM_H
