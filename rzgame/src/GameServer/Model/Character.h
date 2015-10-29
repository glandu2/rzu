#ifndef CHARACTER_H
#define CHARACTER_H

#include "ModelObject.h"
#include "../GameTypes.h"
#include "Database/DbQueryJob.h"
#include <unordered_map>
#include "StatBase.h"
#include "ChangeListener.h"
#include "Inventory.h"

namespace GameServer {

class DB_Character;
class ClientSession;
class Item;

class Character : public ModelObject<Character, 0x8> {
	DECLARE_CLASS(GameServer::Character)
public:
	Character(ClientSession* session, game_sid_t sid, const std::string& account, DB_Character* databaseData);

	void updateStats();
	void sendPacketStats();
	void synchronizeWithClient();

public:
	std::string name;
	float x;
	float y;
	float z;
	int8_t layer;
	float face_direction;
	int32_t hp;
	int32_t mp;
	int32_t maxHp;
	int32_t maxMp;
	int32_t level;
	int32_t jobLevel;
	int64_t exp;
	int64_t jp;
	int32_t sex;
	int32_t race;
	uint32_t skinColor;

	struct ModelInfo {
		int32_t faceId;
		int32_t hairId;
		int32_t armorId;
		int32_t glovesId;
		int32_t bootsId;
	} baseModel;

	int32_t job;
	int32_t stamina;
	int32_t maxStamina;
	int32_t staminaRegen;
	int32_t chaos;
	int32_t maxChaos;
	int32_t prevJobId[3];
	int32_t prevJobLevel[3];
	int32_t tp;
	int32_t huntaholicPoint;
	int32_t huntaholicEntries;
	std::string alias;
	int32_t arena_point;
	int32_t etherealStone;
	int32_t dkCount;
	int32_t pkCount;
	int32_t immoral;
	int32_t permission;
	int32_t channel;
	int32_t status;
	std::string clientInfo;
	std::string quickSlot;
	int64_t gold;

	std::string account;

	StatBase statBase;
	StatBase statBuffs;

	Inventory inventory;

	ClientSession* session;
};

}

#endif // CHARACTER_H
