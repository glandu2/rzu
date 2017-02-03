#include "Inventory.h"
#include "../Database/DB_Item.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_WEAR_INFO.h"
#include "GameClient/TS_SC_ITEM_WEAR_INFO.h"
#include "../ClientSession.h"
#include "Character.h"

namespace GameServer {

Inventory::Inventory(ClientSession* session, Character *character) : session(session), character(character)
{
	equippedItem.fill(nullptr);
}

void Inventory::initializeItems(std::vector<std::unique_ptr<DB_Item>>& dbItems) {
	items.rehash(dbItems.size());
	for(size_t i = 0; i < dbItems.size(); i++) {
		std::unique_ptr<DB_Item>& dbItem = dbItems[i];
		Item* item = new Item(dbItem.get());
		items.insert(std::make_pair(item->handle, std::unique_ptr<Item>(item)));
		if(dbItem->wear_info >= 0 && dbItem->wear_info < (int)equippedItem.size()) {
			equippedItem[dbItem->wear_info] = item;
		}
	}
}

void Inventory::addItem(std::unique_ptr<GameServer::Item> item) {
	Item* itemToUpdate = item.get();
	items.insert(std::make_pair(item->handle, std::unique_ptr<Item>(std::move(item))));

	sendInventoryForItems(&itemToUpdate, 1);
}

void Inventory::equipItem(game_handle_t itemHandle, ItemWearType pos) {
	if(pos >= 0 && pos < equippedItem.size()) {
		auto it = items.find(itemHandle);
		if(it != items.end()) {
			Item *item = it->second.get();

			if(equippedItem[pos])
				sendItemWearInfo(equippedItem[pos], WEAR_NONE);
			equippedItem[pos] = item;
			sendItemWearInfo(equippedItem[pos], pos);
		}
	}
}

void Inventory::unequipItem(ItemWearType pos) {
	if(pos >= 0 && pos < equippedItem.size()) {
		Item *itemToUpdate = equippedItem[pos];
		equippedItem[pos] = nullptr;
		if(itemToUpdate)
			sendItemWearInfo(itemToUpdate, WEAR_NONE);
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

void Inventory::sendInventory() {
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

void Inventory::sendInventoryForItems(Item **items, size_t count) {
	TS_SC_INVENTORY inventory;

	for(size_t i = 0; i < count; i++) {
		TS_ITEM_INFO itemInfo = {0};
		Item* item = items[i];
		if(!item)
			continue;

		item->fillInventoryItem(itemInfo);
		itemInfo.wear_position = getEquipPosition(item);

		inventory.items.push_back(itemInfo);
		if(inventory.items.size() >= 45) {
			session->sendPacket(inventory);
			inventory.items.clear();
		}
	}
	if(inventory.items.size() > 0)
		session->sendPacket(inventory);
}

void Inventory::sendItemWearInfo(Item *item, ItemWearType pos) {
	TS_SC_ITEM_WEAR_INFO wearInfo;
	wearInfo.item_handle = item->handle;
	wearInfo.wear_position = (uint16_t)pos;
	wearInfo.target_handle = character->handle;
	wearInfo.enhance = item->enhance;
	wearInfo.wear_item_elemental_type = item->elemental_effect_type;
	wearInfo.wear_appearance_code = item->appearance_code;
	session->sendPacket(wearInfo);
}

}
