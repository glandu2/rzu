#pragma once

#include "Core/Object.h"
#include "GameData.h"
#include "IPacketInterface.h"
#include "Packet/PacketEpics.h"

class UpdateClientFromGameData : public Object {
public:
	UpdateClientFromGameData(IPacketInterface* clientPacketInterface);

	void removePlayerData(const GameData& gameData);
	void removeCreatureData(const CreatureData& creatureData, bool isLocalPlayer);
	void addPlayerData(const GameData& gameData);

private:
	void sendLoginResultToClient(const TS_SC_LOGIN_RESULT* loginResult);
	void sendPartyInfo(const LocalPlayerData::PartyInfo* party);
	void sendCreatureData(const CreatureData& creatureData, bool isForLocalPlayer);

private:
	IPacketInterface* clientPacketInterface;
};
