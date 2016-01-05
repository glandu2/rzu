#ifndef INVENTORY_H
#define INVENTORY_H

#include "../GameTypes.h"
#include <memory>
#include <unordered_map>
#include <array>
#include "Item.h"

namespace GameServer {

class ClientSession;
class Character;

class Inventory
{
public:

	enum ItemWearType {
		WEAR_NONE = -1,
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
		WEAR_MAX = 28
	};
public:
	Inventory(ClientSession* session, Character* character);

	void initializeItems(std::vector<std::unique_ptr<DB_Item> > &dbItems);
	void addItem(std::unique_ptr<Item> item);
	void equipItem(game_handle_t itemHandle, ItemWearType pos);
	void unequipItem(ItemWearType pos);
	Item* getEquipedItem(ItemWearType pos);
	ItemWearType getEquipPosition(Item* item);

	void sendInventory();
	void sendInventoryForItems(Item** items, size_t count);
	void sendItemWearInfo(Item* item, ItemWearType pos);

private:
	std::unordered_map<game_handle_t, const std::unique_ptr<Item>> items;
	std::array<Item*, WEAR_MAX> equippedItem;

	ClientSession* session;
	Character* character;
};

}

#endif // INVENTORY_H
