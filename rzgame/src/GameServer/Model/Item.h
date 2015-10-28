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

	enum ItemWearType {
		WEAR_CANTWEAR = 0xFFFFFFFF,
		WEAR_NONE = 0xFFFFFFFF,
		WEAR_RIGHTHAND = 0,
		WEAR_LEFTHAND = 1,
		WEAR_ARMOR = 2,
		WEAR_HELM = 3,
		WEAR_GLOVE = 4,
		WEAR_BOOTS = 5,
		WEAR_BELT = 6,
		WEAR_MANTLE = 7,
		WEAR_ARMULET = 8,
		WEAR_RING = 9,
		WEAR_SECOND_RING = 10,
		WEAR_EAR = 11,
		WEAR_FACE = 12,
		WEAR_HAIR = 13,
		WEAR_DECO_WEAPON = 14,
		WEAR_DECO_SHIELD = 15,
		WEAR_DECO_ARMOR = 16,
		WEAR_DECO_HELM = 17,
		WEAR_DECO_GLOVE = 18,
		WEAR_DECO_BOOTS = 19,
		WEAR_DECO_MANTLE = 20,
		WEAR_DECO_SHOULDER = 21,
		WEAR_RIDE_ITEM = 22,
		WEAR_BAG_SLOT = 23,
		WEAR_SPARE_RIGHTHAND = 24,
		WEAR_SPARE_LEFTHAND = 25,
		WEAR_SPARE_DECO_WEAPON = 26,
		WEAR_SPARE_DECO_SHIELD = 27,
		WEAR_TWOFINGER_RING = 94,
		WEAR_TWOHAND = 99,
		WEAR_SKILL = 100,
		WEAR_SUMMON_ONLY = 200,
	};


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
	int32_t wear_info;
	int32_t socket[4];
	int32_t awaken_sid;
	int32_t remain_time;
	int8_t elemental_effect_type;
	int32_t elemental_effect_attack_point;
	int32_t elemental_effect_magic_point;
	int32_t appearance_code;

	Timer elemental_effect_timer;
	Timer expire_timer;
};

}

#endif // ITEM_H
