#include "GameClientSession.h"
#include "GlobalConfig.h"
#include "PacketHandleConverter.h"
#include <numeric>
#include <stdarg.h>

#include "PacketIterator.h"

#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_SC_CHANGE_NAME.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_GAME_TIME.h"
#include "GameClient/TS_SC_HPMP.h"
#include "GameClient/TS_SC_LEAVE.h"
#include "GameClient/TS_SC_PROPERTY.h"
#include "GameClient/TS_SC_RESULT.h"
#include "GameClient/TS_SC_SET_TIME.h"
#include "GameClient/TS_SC_SKIN_INFO.h"
#include "GameClient/TS_SC_UPDATE_ITEM_COUNT.h"
#include "GameClient/TS_SC_WARP.h"
#include "GameClient/TS_TIMESYNC.h"

/* TODO:
 * handle chat, guild, party, friends
 */
GameClientSession::GameClientSession()
    : EncryptedSession<PacketSession>(
          SessionType::GameClient, SessionPacketOrigin::Server, CONFIG_GET()->generalConfig.epic.get()) {
	log(LL_Info, "Client connected to game client session\n");
}

void GameClientSession::sendCharMessage(const char* msg, ...) {
	TS_SC_CHAT chatMessage;
	va_list args;

	va_start(args, msg);
	Utils::stringFormatv(chatMessage.message, msg, args);
	va_end(args);

	log(LL_Info, "Sending chat message to client: %s\n", chatMessage.message.c_str());

	chatMessage.szSender = "rzcr";
	chatMessage.type = CHAT_WHISPER;

	sendPacket(chatMessage);
}

bool GameClientSession::isKnownLocalPlayer(std::string_view playerName) {
	for(auto& connection : connections) {
		if(connection.getPlayerName() == playerName)
			return true;
	}
	return false;
}

ar_handle_t GameClientSession::convertHandle(ConnectionToServer* targetServer, ar_handle_t val) {
	if(targetServer == connectionToServer && val == targetServer->getLocalPlayerServerHandle()) {
		return getLocalPlayerClientHandle();
	} else if(val == getLocalPlayerClientHandle()) {
		return targetServer->getLocalPlayerServerHandle();
	}
	return val;
}

template<class Packet> void GameClientSession::convertPacketHandles(ConnectionToServer* targetServer, Packet& packet) {
	auto handleConverterLambda = [this, targetServer](const char* fieldName, ar_handle_t& val) {
		ar_handle_t newVal = convertHandle(targetServer, val);
		if(val != newVal) {
			log(LL_Info,
			    "Converted handle from %x to %x in field %s::%s\n",
			    val.get(),
			    newVal.get(),
			    Packet::getName(),
			    fieldName);
		}
		val = newVal;
	};
	PacketHandleConverter<decltype(handleConverterLambda)&> handleConverter{packetVersion, handleConverterLambda};
	packet.deserialize(&handleConverter);
}

template<class Packet> void GameClientSession::sendServerPacketToClient(const Packet& packet) {
	Packet packetCopy = packet;
	sendServerPacketToClient(packetCopy);
}

template<class Packet> void GameClientSession::sendClientPacketToServer(const Packet& packet) {
	Packet packetCopy = packet;
	sendClientPacketToServer(packetCopy);
}

template<class Packet> void GameClientSession::sendServerPacketToClient(Packet& packet) {
	convertPacketHandles(connectionToServer, packet);
	sendPacket(packet);
}

void GameClientSession::sendServerPacketToClient(TS_SC_CHAT& packet) {
	if(packet.type >= CHAT_PARTY_SYSTEM) {
		Utils::stringReplaceAll(packet.message, std::to_string(localPlayerClientHandle.get()), "CLIENTHNDL");
		Utils::stringReplaceAll(
		    packet.message, std::to_string(connectionToServer->getLocalPlayerServerHandle().get()), "SERVERHNDL");
		Utils::stringReplaceAll(packet.message, "SERVERHNDL", std::to_string(localPlayerClientHandle.get()));
		Utils::stringReplaceAll(
		    packet.message, "CLIENTHNDL", std::to_string(connectionToServer->getLocalPlayerServerHandle().get()));
	}
	sendPacket(packet);
}

template<class Packet> void GameClientSession::sendClientPacketToServer(Packet& packet) {
	if(!connectionToServer || !controlledServer)
		return;

	convertPacketHandles(controlledServer, packet);
	controlledServer->onClientPacketReceived(packet);
}

template<class Packet> struct GameClientSession::ConvertHandlesFunctor {
	template<class SenderLambda>
	bool operator()(GameClientSession* self,
	                const TS_MESSAGE* packet,
	                packet_version_t version,
	                const SenderLambda& sendPacketLambda) {
		Packet updatedPacket;

		MessageBuffer buffer(packet, packet->size, version);
		updatedPacket.deserialize(&buffer);
		if(buffer.checkPacketFinalSize()) {
			sendPacketLambda(updatedPacket);
		} else {
			self->log(LL_Error, "Can't parse packet id %d\n", packet->id);
		}

		return true;
	}
};

// In case of a serialization error
void GameClientSession::onServerPacket(ConnectionToServer* connection, const TS_MESSAGE* packet) {
	if(packet->id == TS_SC_LOGIN_RESULT::getId(packetVersion)) {
		if(!isClientLogged) {
			localPlayerClientHandle = connection->getLocalPlayerServerHandle();
			sendPacket(packet);
			isClientLogged = true;
		} else {
			// client is already logged on, don't send TS_SC_LOGIN_RESULT again
			sendCharMessage("Game server reconnected");
		}
	} else if(isClientLogged || packet->id == TS_TIMESYNC::packetID) {
		bool dummy;
		bool packetProcessed = processPacket<ConvertHandlesFunctor>(
		    SessionType::GameClient,
		    SessionPacketOrigin::Server,
		    packetVersion,
		    packet->id,
		    dummy,
		    this,
		    packet,
		    packetVersion,
		    [this](auto& packetToSend) { sendServerPacketToClient(packetToSend); });

		if(!packetProcessed) {
			// fallback
			log(Object::LL_Debug, "Can't send packet id %d (unknown packet)\n", packet->id);
			sendPacket(packet);
		}
	}
}

void GameClientSession::onServerDisconnected(ConnectionToServer*, GameData&& gameData) {
	// Remove the player and all inventory items
	removePlayerData(gameData);
}

void GameClientSession::removePlayerData(const GameData& gameData) {
	log(LL_Info, "Removing player and items from the client\n");

	// If no login result, no data to remove
	if(!gameData.localPlayer.loginResult)
		return;

	// TODO: handle Chat packets: parties, guild, other ?

	removeCreatureData(gameData.localPlayer.creatureData, true);

	for(const auto& it : gameData.localPlayer.summonInfosByCardHandle) {
		TS_SC_REMOVE_SUMMON_INFO packet;
		packet.card_handle = it.first;
		sendServerPacketToClient(packet);
	}

	for(const auto& it : gameData.localPlayer.petInfosByCardHandle) {
		TS_SC_REMOVE_PET_INFO packet;
		packet.handle = it.first;
		sendServerPacketToClient(packet);
	}

	for(const auto& it : gameData.localPlayer.skillCardInfoByItemHandle) {
		TS_SC_SKILLCARD_INFO packet;
		packet.item_handle = it.first;
		packet.target_handle = ar_handle_t(0);
		sendServerPacketToClient(packet);
	}

	for(const auto& it : gameData.localPlayer.itemsByHandle) {
		ar_handle_t handle = it.first;
		TS_SC_UPDATE_ITEM_COUNT deleteItemMessage;
		deleteItemMessage.item_handle = handle;
		deleteItemMessage.count = 0;
		sendServerPacketToClient(deleteItemMessage);
	}

	for(auto& party : gameData.localPlayer.partyInfoByName) {
		TS_SC_CHAT packet;
		packet.szSender = "@PARTY";
		packet.type = CHAT_PARTY_SYSTEM;
		Utils::stringFormat(packet.message, "DESTROY|%s|%d|", party.second.partyName.c_str(), 0);
		sendServerPacketToClient(packet);
	}

	for(const auto& it : gameData.creaturesData) {
		ar_handle_t handle = it.first;
		if(handle == gameData.localPlayer.loginResult->handle) {
			log(LL_Warning, "Skipping sending TS_SC_LEAVE with client's local player's handle\n");
			continue;
		}
		removeCreatureData(it.second, false);
	}
}

void GameClientSession::removeCreatureData(const CreatureData& creatureData, bool isLocalPlayer) {
	if(isLocalPlayer) {
		for(const auto& skill : creatureData.activeSkillsBySkillId) {
			TS_SC_SKILL skillPacket = skill.second;
			skillPacket.type = ST_Cancel;
			sendServerPacketToClient(skillPacket);
		}

		for(const auto& aura : creatureData.aurasBySkillId) {
			auto packet = aura.second;
			packet.status = 0;
			sendServerPacketToClient(packet);
		}

		for(const auto& state : creatureData.statesByHandle) {
			TS_SC_STATE packet = state.second;
			packet.state_level = 0;
			sendServerPacketToClient(packet);
		}

		for(const auto& it : creatureData.itemWearByPosition) {
			TS_SC_ITEM_WEAR_INFO packet = it.second;
			packet.wear_position = -1;
			sendServerPacketToClient(packet);
		}
	} else {
		TS_SC_LEAVE leaveMessage;
		leaveMessage.handle = creatureData.enterInfo.handle;
		sendServerPacketToClient(leaveMessage);
	}
}

void GameClientSession::addPlayerData(const GameData& gameData) {
	log(LL_Info, "Adding player and items to the client\n");

	// If no login result, we are not yet connected
	if(!gameData.localPlayer.loginResult)
		return;

	// Send local player data
	sendLoginResultToClient(&gameData.localPlayer.loginResult.value());

	//	for(const auto& packet : gameData.localPlayer.chatPropertiesBySender)
	//		sendServerPacketToClient(packet.second);

	if(!gameData.localPlayer.itemsByHandle.empty()) {
		TS_SC_INVENTORY inventoryPacket;
		auto it = gameData.localPlayer.itemsByHandle.begin();
		while(it != gameData.localPlayer.itemsByHandle.end()) {
			inventoryPacket.items.push_back(it->second);

			// If packet cannot have a additional item without going above max packet size,
			// send it and clear inventory packet's items
			if(inventoryPacket.getSize(packetVersion) + it->second.getSize(packetVersion) > 16384) {
				sendServerPacketToClient(inventoryPacket);
				inventoryPacket.items.clear();
			}
			++it;
		}
		if(!inventoryPacket.items.empty()) {
			sendServerPacketToClient(inventoryPacket);
		}
	}

	for(const auto& packet : gameData.localPlayer.summonInfosByCardHandle)
		sendServerPacketToClient(packet.second);
	for(const auto& packet : gameData.localPlayer.petInfosByCardHandle)
		sendServerPacketToClient(packet.second);

	if(gameData.localPlayer.equipSummon)
		sendServerPacketToClient(gameData.localPlayer.equipSummon.value());
	if(gameData.localPlayer.gold)
		sendServerPacketToClient(gameData.localPlayer.gold.value());
	if(gameData.localPlayer.beltSlotInfo)
		sendServerPacketToClient(gameData.localPlayer.beltSlotInfo.value());
	if(gameData.localPlayer.battleArenaPenaltyInfo)
		sendServerPacketToClient(gameData.localPlayer.battleArenaPenaltyInfo.value());

	if(gameData.localPlayer.questList)
		sendServerPacketToClient(gameData.localPlayer.questList.value());
	if(gameData.localPlayer.titleList)
		sendServerPacketToClient(gameData.localPlayer.titleList.value());
	if(gameData.localPlayer.titleConditionList)
		sendServerPacketToClient(gameData.localPlayer.titleConditionList.value());
	if(gameData.localPlayer.titleRemainTime)
		sendServerPacketToClient(gameData.localPlayer.titleRemainTime.value());
	if(gameData.localPlayer.subTitle)
		sendServerPacketToClient(gameData.localPlayer.subTitle.value());

	sendCreatureData(gameData.localPlayer.creatureData, true);

	for(const auto& packet : gameData.localPlayer.skillCardInfoByItemHandle)
		sendServerPacketToClient(packet.second);

	if(gameData.localPlayer.energy)
		sendServerPacketToClient(gameData.localPlayer.energy.value());
	if(gameData.localPlayer.hideEquipInfo)
		sendServerPacketToClient(gameData.localPlayer.hideEquipInfo.value());

	if(gameData.localPlayer.status)
		sendServerPacketToClient(gameData.localPlayer.status.value());

	for(auto& party : gameData.localPlayer.partyInfoByName) {
		sendPartyInfo(&party.second);
	}

	// Send other creatures data
	for(const auto& creature : gameData.creaturesData)
		sendCreatureData(creature.second, false);
}

void GameClientSession::sendLoginResultToClient(const TS_SC_LOGIN_RESULT* loginResult) {
	TS_SC_WARP warpPacket;
	warpPacket.x = loginResult->x;
	warpPacket.y = loginResult->y;
	warpPacket.z = loginResult->z;
	warpPacket.layer = loginResult->layer;
	sendServerPacketToClient(warpPacket);

	//	TS_SC_MOVE movePacket;
	//	movePacket.speed = INT8_MAX;
	//	movePacket.tlayer = loginResult->layer;
	//	movePacket.handle = loginResult->handle;
	//	movePacket.start_time = ar_time_t(0);
	//	movePacket.move_infos.push_back(MOVE_INFO{loginResult->x, loginResult->y});
	//	sendServerPacketToClient(movePacket);

	TS_SC_HPMP hpmpPacket{};
	hpmpPacket.handle = loginResult->handle;
	hpmpPacket.need_to_display = false;  // if true and add_xx not 0, display in client as damage
	hpmpPacket.hp = loginResult->hp;
	hpmpPacket.mp = loginResult->mp;
	hpmpPacket.max_hp = loginResult->max_hp;
	hpmpPacket.max_mp = loginResult->max_mp;
	sendServerPacketToClient(hpmpPacket);

	std::initializer_list<std::pair<std::string, int64_t>> properties = {
	    {"sex", loginResult->sex},
	    {"race", loginResult->race},
	    {"hairId", loginResult->hairId},
	    {"guild_id", loginResult->guild_id},
	    {"back_board", loginResult->back_board},  // TODO: not sure
	};

	for(const auto& property : properties) {
		TS_SC_PROPERTY propertyPacket;
		propertyPacket.handle = loginResult->handle;
		propertyPacket.name = property.first;
		propertyPacket.is_number = true;
		propertyPacket.value = property.second;
		sendServerPacketToClient(hpmpPacket);
	}

	TS_SC_SKIN_INFO skinInfoPacket;
	skinInfoPacket.handle = loginResult->handle;
	skinInfoPacket.nSkinColor = loginResult->skin_color;
	sendServerPacketToClient(skinInfoPacket);

	TS_SC_CHANGE_NAME changeNamePacket;
	changeNamePacket.handle = loginResult->handle;
	changeNamePacket.name = loginResult->name;
	sendServerPacketToClient(changeNamePacket);
}

void GameClientSession::sendPartyInfo(const LocalPlayerData::PartyInfo* party) {
	TS_SC_CHAT packet;
	packet.szSender = "@PARTY";
	packet.type = CHAT_PARTY_SYSTEM;

	for(auto& member : party->membersByName) {
		Utils::stringFormat(packet.message,
		                    "MINFO|%u|%s|%d|%d|%d|%d|%d|%d|%d|",
		                    member.second.handle,
		                    member.second.name.c_str(),
		                    member.second.level,
		                    member.second.jobId,
		                    member.second.hpPercentage,
		                    member.second.mpPercentage,
		                    member.second.x,
		                    member.second.y,
		                    member.second.creature);
		sendServerPacketToClient(packet);
	}

	Utils::stringFormat(packet.message,
	                    "PINFO|%d|%s|%s|%d|%d|%d|%d|",
	                    party->partyId,
	                    party->partyName.c_str(),
	                    party->leaderDisplayName.c_str(),
	                    party->shareMode,
	                    party->maxLevel,
	                    party->minLevel,
	                    party->partyType);
	for(auto& member : party->membersByName) {
		std::string buffer;
		Utils::stringFormat(buffer,
		                    "%u|%s|%d|%d|%d|%d|%d|%d|%d|",
		                    member.second.handle,
		                    member.second.name.c_str(),
		                    member.second.level,
		                    member.second.jobId,
		                    member.second.hpPercentage,
		                    member.second.mpPercentage,
		                    member.second.x,
		                    member.second.y,
		                    member.second.creature);
		packet.message += buffer;
	}
	sendServerPacketToClient(packet);

	if(!party->summonByPlayerHandle.empty()) {
		Utils::stringFormat(packet.message, "SINFO|%d|%d|", party->partyId, (int) party->summonByPlayerHandle.size());

		for(auto& summon : party->summonByPlayerHandle) {
			std::string buffer;
			Utils::stringFormat(buffer,
			                    "%u|%u|%u|",
			                    summon.second.playerHandle,
			                    summon.second.mainSummonHandle,
			                    summon.second.subSummonHandle);
			packet.message += buffer;
		}
		sendServerPacketToClient(packet);
	}

	packet.message = "ARENAMODE|-1|0";
	sendServerPacketToClient(packet);
}

void GameClientSession::sendCreatureData(const CreatureData& creatureData, bool isForLocalPlayer) {
	if(!isForLocalPlayer)
		sendServerPacketToClient(creatureData.enterInfo);

	if(!creatureData.skillsById.empty()) {
		TS_SC_SKILL_LIST skillListPacket;
		skillListPacket.target = creatureData.enterInfo.handle;
		// 0: UPDATE, 1: REFRESH (reset + add)
		skillListPacket.modification_type = 1;
		for(const auto& packet : creatureData.skillsById)
			skillListPacket.skills.push_back(packet.second);
		sendServerPacketToClient(skillListPacket);
	}

	if(!creatureData.skillsById.empty()) {
		TS_SC_ADDED_SKILL_LIST skillListPacket;
		skillListPacket.target = creatureData.enterInfo.handle;
		for(const auto& packet : creatureData.addedSkillsById)
			skillListPacket.skills.push_back(packet.second);
		sendServerPacketToClient(skillListPacket);
	}

	for(const auto& property : creatureData.stringPropertiesByName) {
		TS_SC_PROPERTY propertyMessage;
		propertyMessage.handle = creatureData.enterInfo.handle;
		propertyMessage.name = property.first;
		propertyMessage.is_number = false;
		propertyMessage.string_value = property.second;
		sendServerPacketToClient(propertyMessage);
	}

	for(const auto& property : creatureData.intPropertiesByName) {
		TS_SC_PROPERTY propertyMessage;
		propertyMessage.handle = creatureData.enterInfo.handle;
		propertyMessage.name = property.first;
		propertyMessage.is_number = true;
		propertyMessage.value = property.second;
		sendServerPacketToClient(propertyMessage);
	}

	if(creatureData.wearInfo)
		sendServerPacketToClient(creatureData.wearInfo.value());

	for(const auto& packet : creatureData.itemWearByPosition)
		sendServerPacketToClient(packet.second);

	if(creatureData.level)
		sendServerPacketToClient(creatureData.level.value());
	if(creatureData.exp)
		sendServerPacketToClient(creatureData.exp.value());
	if(creatureData.detectRange)
		sendServerPacketToClient(creatureData.detectRange.value());

	for(const auto& packet : creatureData.statesByHandle)
		sendServerPacketToClient(packet.second);

	for(const auto& packet : creatureData.aurasBySkillId)
		sendServerPacketToClient(packet.second);

	if(creatureData.mainTitle)
		sendServerPacketToClient(creatureData.mainTitle.value());

	for(const auto& packet : creatureData.activeSkillsBySkillId)
		sendServerPacketToClient(packet.second);

	if(creatureData.statInfoTotal)
		sendServerPacketToClient(creatureData.statInfoTotal.value());

	if(creatureData.statInfoByItems)
		sendServerPacketToClient(creatureData.statInfoByItems.value());

	if(creatureData.unsummonNotice)
		sendServerPacketToClient(creatureData.unsummonNotice.value());

	if(creatureData.skinInfo)
		sendServerPacketToClient(creatureData.skinInfo.value());

	if(creatureData.activeMove)
		sendServerPacketToClient(creatureData.activeMove.value());
}

void GameClientSession::activateConnectionToServer(ConnectionToServer* connectionToServer) {
	if(this->connectionToServer)
		removePlayerData(this->connectionToServer->attachClient(nullptr));

	this->connectionToServer = connectionToServer;
	this->controlledServer = connectionToServer;

	if(this->connectionToServer)
		addPlayerData(this->connectionToServer->attachClient(this));
}

void GameClientSession::spawnConnectionToServer(const std::string& account,
                                                const std::string& password,
                                                const std::string& playername) {
	log(LL_Info, "Spawning server connection %d\n", (int) connections.size());

	connections.emplace_back(account, password, playername);
	ConnectionToServer* connection = &connections.back();

	if(!connectionToServer)
		activateConnectionToServer(connection);

	if(CONFIG_GET()->trafficDump.enableServer.get())
		connection->setPacketLogger(getStream()->getPacketLogger());
	else
		connection->setPacketLogger(nullptr);
	connection->connect();
}

EventChain<PacketSession> GameClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	if(packet->id == 9999)
		return EncryptedSession<PacketSession>::onPacketReceived(packet);

	bool forwardPacket = true;

	packet_type_id_t packetType = PacketMetadata::convertPacketIdToTypeId(
	    packet->id, SessionType::GameClient, SessionPacketOrigin::Client, packetVersion);
	switch(packetType) {
		case TS_CS_ACCOUNT_WITH_AUTH::packetID:
			packet->process(this, &GameClientSession::onAccountWithAuth, packetVersion);
			forwardPacket = false;
			break;

		case TS_CS_CHARACTER_LIST::packetID:
			packet->process(this, &GameClientSession::onCharacterList, packetVersion);
			forwardPacket = false;
			break;

		case TS_CS_LOGIN::packetID:
			packet->process(this, &GameClientSession::onLogin, packetVersion);
			forwardPacket = false;
			break;

		case TS_CS_CHAT_REQUEST::packetID:
			packet->process(this, &GameClientSession::onChatRequest, packetVersion, &forwardPacket);
			break;

		case TS_CS_TARGETING::packetID:
			packet->process(this, &GameClientSession::onTargeting, packetVersion);
			break;
	}

	if(forwardPacket && connectionToServer && (isClientLogged || packet->id == TS_TIMESYNC::packetID)) {
		bool dummy;
		bool packetProcessed = processPacket<ConvertHandlesFunctor>(
		    SessionType::GameClient,
		    SessionPacketOrigin::Client,
		    packetVersion,
		    packet->id,
		    dummy,
		    this,
		    packet,
		    packetVersion,
		    [this](auto& packetToSend) { sendClientPacketToServer(packetToSend); });

		if(!packetProcessed) {
			log(Object::LL_Debug, "Can't send packet id %d (unknown packet)\n", packet->id);
			connectionToServer->onClientPacketReceived(packet);
		}
	}

	return EncryptedSession<PacketSession>::onPacketReceived(packet);
}

void GameClientSession::onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH* packet) {
	TS_SC_RESULT result;

	log(LL_Info, "Received account with auth with account\n");

	result.request_msg_id = TS_CS_ACCOUNT_WITH_AUTH::packetID;
	result.result = 0;
	result.value = 0;

	sendPacket(result);
}

void GameClientSession::onCharacterList(const TS_CS_CHARACTER_LIST* packet) {
	TS_SC_CHARACTER_LIST fakeCharacterList{};

	log(LL_Info, "Received character list query\n");

	if(connectionToServer)
		fakeCharacterList.current_server_time = connectionToServer->getGameTime();
	else
		fakeCharacterList.current_server_time = 0;

	fakeCharacterList.last_character_idx = 0;
	fakeCharacterList.characters.emplace_back();
	LOBBY_CHARACTER_INFO& character = fakeCharacterList.characters.back();

	character.sex = 2;
	character.race = 4;
	character.model_id[0] = 103;
	character.model_id[1] = 206;
	character.model_id[2] = 301;
	character.model_id[3] = 401;
	character.model_id[4] = 501;
	character.hair_color_index = 1;
	character.texture_id = 1;
	character.wear_info[0] = 106100;
	character.wear_info[2] = 220109;
	character.wear_info[23] = 490001;
	character.level = 1;
	character.job = 200;
	character.job_level = 1;
	if(connectionToServer)
		character.name = connectionToServer->getPlayerName();
	else
		character.name = "rzclientreconnect";
	character.skin_color = 8487040;
	character.szCreateTime = "2016-02-28 19:38:41";
	character.szDeleteTime = "9999-12-31 00:00:00";

	sendPacket(fakeCharacterList);
}

void GameClientSession::onLogin(const TS_CS_LOGIN* packet) {
	log(LL_Info, "Received client's login\n");

	timeSync.clear();

	if(connections.empty())
		spawnConnectionToServer(CONFIG_GET()->server.account.get(),
		                        CONFIG_GET()->server.password.get(),
		                        CONFIG_GET()->server.playerName.get());
}

void GameClientSession::onChatRequest(const TS_CS_CHAT_REQUEST* packet, bool* forwardPacket) {
	if(packet->szTarget == "rzcr" && packet->type == CHAT_WHISPER) {
		std::vector<std::string> args = Utils::parseCommand(packet->message);

		*forwardPacket = false;

		if(args.empty())
			return;

		log(LL_Info, "Received command from client: %s\n", packet->message.c_str());

		if(args[0] == "help") {
			sendCharMessage("Available commands: add, remove, list, switch");
		} else if(args[0] == "add") {
			if(args.size() == 4) {
				spawnConnectionToServer(args[1], args[2], args[3]);
				sendCharMessage("Connecting character %s", args[3].c_str());
			} else {
				sendCharMessage("Usage: add (account) (password) (character)");
			}
		} else if(args[0] == "remove") {
			if(args.size() == 2) {
				std::list<ConnectionToServer>::iterator connectionIt = connections.end();
				for(auto it = connections.begin(); it != connections.end(); ++it) {
					if(it->getPlayerName() == args[1]) {
						connectionIt = it;
						break;
					}
				}
				if(connectionIt != connections.end()) {
					ConnectionToServer* newConnection = &(*connectionIt);
					sendCharMessage("Disconnecting character %s", newConnection->getPlayerName().c_str());
					if(newConnection == connectionToServer)
						activateConnectionToServer(nullptr);
					newConnection->closeSession();
					connections.erase(connectionIt);
				} else {
					sendCharMessage("Error: no connection with player %s", args[1].c_str());
				}
			} else {
				sendCharMessage("Usage: remove (character)");
			}
		} else if(args[0] == "list") {
			sendCharMessage("Connected players:");
			for(auto& connection : connections) {
				sendCharMessage(
				    "%s (handle 0x%x)", connection.getPlayerName().c_str(), connection.getLocalPlayerServerHandle());
			}
		} else if(args[0] == "switch") {
			if(args.size() == 2) {
				ConnectionToServer* newConnection = nullptr;
				for(auto& connection : connections) {
					if(connection.getPlayerName() == args[1]) {
						newConnection = &connection;
						break;
					}
				}
				if(newConnection) {
					if(newConnection == connectionToServer)
						sendCharMessage("Warning: player %s was already active player", args[1].c_str());
					sendCharMessage("Switching to player %s", args[1].c_str());
					activateConnectionToServer(newConnection);
				} else {
					sendCharMessage("Error: no connection with player %s", args[1].c_str());
				}
			} else if(args.size() == 1) {
				if(lastTargetedPlayerClientHandle.get()) {
					ar_handle_t lastTargetedPlayerServerHandle =
					    convertHandle(connectionToServer, lastTargetedPlayerClientHandle);
					ConnectionToServer* newConnection = nullptr;

					for(auto& connection : connections) {
						if(connection.getLocalPlayerServerHandle() == lastTargetedPlayerServerHandle) {
							newConnection = &connection;
							break;
						}
					}

					if(newConnection) {
						if(newConnection == connectionToServer) {
							sendCharMessage("Warning: player %s was already active player",
							                newConnection->getPlayerName().c_str());
						}
						sendCharMessage("Switching to player %s", newConnection->getPlayerName().c_str());
						activateConnectionToServer(newConnection);
					} else {
						sendCharMessage("Error: no connection with targeted player (handle 0x%x)",
						                lastTargetedPlayerServerHandle);
					}
				}
			} else {
				sendCharMessage("Usage: switch (character) (character is optional if a player is selected)");
			}
		} else if(args[0] == "control") {
			if(args.size() == 2) {
				ConnectionToServer* newConnection = nullptr;
				for(auto& connection : connections) {
					if(connection.getPlayerName() == args[1]) {
						newConnection = &connection;
						break;
					}
				}
				if(newConnection) {
					if(newConnection == controlledServer)
						sendCharMessage("Warning: player %s was already controlled player", args[1].c_str());
					sendCharMessage("Switching control to player %s", args[1].c_str());
					controlledServer = newConnection;
				} else {
					sendCharMessage("Error: no connection with player %s", args[1].c_str());
				}
			} else if(args.size() == 1) {
				if(lastTargetedPlayerClientHandle.get()) {
					ar_handle_t lastTargetedPlayerServerHandle =
					    convertHandle(connectionToServer, lastTargetedPlayerClientHandle);
					ConnectionToServer* newConnection = nullptr;

					for(auto& connection : connections) {
						if(connection.getLocalPlayerServerHandle() == lastTargetedPlayerServerHandle) {
							newConnection = &connection;
							break;
						}
					}

					if(newConnection) {
						if(newConnection == controlledServer) {
							sendCharMessage("Warning: player %s was already controlled player",
							                newConnection->getPlayerName().c_str());
						}
						sendCharMessage("Switching control to player %s", newConnection->getPlayerName().c_str());
						controlledServer = newConnection;
					} else {
						sendCharMessage("Error: no connection with targeted player (handle 0x%x)",
						                lastTargetedPlayerServerHandle);
					}
				}
			} else {
				sendCharMessage("Usage: control (character) (character is optional if a player is selected)");
			}
		}
	}
}

void GameClientSession::onTargeting(const TS_CS_TARGETING* packet) {
	lastTargetedPlayerClientHandle = packet->target;
}

GameClientSession::~GameClientSession() {
	log(LL_Info, "Client disconnected, closing server connections\n");
	activateConnectionToServer(nullptr);
	for(auto& connection : connections)
		connection.abortSession();
}
