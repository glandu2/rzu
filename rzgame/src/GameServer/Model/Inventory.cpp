#include "Inventory.h"
#include "../Database/DB_Item.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_EQUIP_SUMMON.h"
#include "GameClient/TS_SC_WEAR_INFO.h"
#include "../ClientSession.h"

namespace GameServer {

Inventory::Inventory()
{
	equippedItem.fill(nullptr);
}

void Inventory::addItem(std::vector<std::unique_ptr<DB_Item>>& dbItems) {
	items.rehash(dbItems.size());
	for(size_t i = 0; i < dbItems.size(); i++) {
		std::unique_ptr<DB_Item>& dbItem = dbItems[i];
		Item* item = new Item(dbItem.get());
		items.insert(std::make_pair(item->handle, std::unique_ptr<Item>(item)));
		if(dbItem->wear_info >= 0 && dbItem->wear_info < equippedItem.size()) {
			equippedItem[dbItem->wear_info] = item;
		}
	}
}

void Inventory::addItem(std::unique_ptr<GameServer::Item> item) {
	items.insert(std::make_pair(item->handle, std::unique_ptr<Item>(std::move(item))));
}

void Inventory::equipItem(game_handle_t itemHandle, ItemWearType pos) {
	if(pos >= 0 && pos < equippedItem.size()) {
		auto it = items.find(itemHandle);
		if(it != items.end()) {
			Item* item = it->second.get();
			equippedItem[pos] = item;
		}
	}
}

void Inventory::unequipItem(ItemWearType pos) {
	if(pos >= 0 && pos < equippedItem.size()) {
		equippedItem[pos] = nullptr;
	}
}

Item *Inventory::getEquipedItem(Inventory::ItemWearType pos) {
	if(pos >= 0 && pos < equippedItem.size()) {
		return equippedItem[pos];
	}

	return nullptr;
}

Inventory::ItemWearType Inventory::getEquipPosition(Item *item) {
	for(size_t i = 0; i < equippedItem.size(); i++) {
		if(equippedItem[i] == item)
			return (ItemWearType)i;
	}

	return WEAR_NONE;
}

void Inventory::sendInventory(ClientSession *session) {
	TS_SC_INVENTORY inventory;
	auto it = items.begin();
	auto itEnd = items.end();
	for(; it != itEnd; ++it) {
		TS_ITEM_INFO itemInfo = {0};
		Item* item = it->second.get();

		item->fillInventoryItem(itemInfo);
		itemInfo.wear_position = getEquipPosition(item);

		inventory.items.push_back(itemInfo);
		if(inventory.items.size() >= 45) {
			session->sendPacket(inventory);
			inventory.items.clear();
		}
	}
	if(inventory.items.size() > 0 || items.size() == 0)
		session->sendPacket(inventory);
}

}
