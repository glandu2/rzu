#pragma once

#include "ConnectionToServer.h"
#include "NetSession/EncryptedSession.h"
#include "NetSession/PacketSession.h"
#include <list>
#include <stdint.h>
#include <string>
#include <vector>

class AuthClientSession;

struct TS_CS_ACCOUNT_WITH_AUTH;
struct TS_CS_CHARACTER_LIST;
struct TS_CS_LOGIN;
struct TS_TIMESYNC;
struct TS_CS_GAME_TIME;
struct TS_CS_CHAT_REQUEST;
struct TS_CS_TARGETING;

class GameClientSession : public EncryptedSession<PacketSession> {
	DECLARE_CLASS(GameClientSession)

public:
	GameClientSession();

	void sendCharMessage(const char* msg, ...);

	ar_handle_t getLocalPlayerClientHandle() { return localPlayerClientHandle; }
	bool getIsClientLogged() { return isClientLogged; }
	bool isKnownLocalPlayer(std::string_view playerName);

	void onServerPacket(ConnectionToServer* connectionToServer, const TS_MESSAGE* packet);
	void onServerDisconnected(ConnectionToServer* connectionToServer, GameData&& gameData);

private:
	void removePlayerData(const GameData& gameData);
	void removeCreatureData(const CreatureData& creatureData, bool isLocalPlayer);
	void addPlayerData(const GameData& gameData);
	void sendLoginResultToClient(const TS_SC_LOGIN_RESULT* loginResult);
	void sendPartyInfo(const LocalPlayerData::PartyInfo* party);
	void sendCreatureData(const CreatureData& creatureData, bool isForLocalPlayer);

	void activateConnectionToServer(ConnectionToServer* connectionToServer);
	void spawnConnectionToServer(const std::string& account,
	                             const std::string& password,
	                             const std::string& playername);

	ar_handle_t convertHandle(ConnectionToServer* targetServer, ar_handle_t val);
	template<class Packet> struct ConvertHandlesFunctor;
	template<class Packet> void convertPacketHandles(ConnectionToServer* targetServer, Packet& packet);
	template<class Packet> void sendServerPacketToClient(const Packet& packet);
	template<class Packet> void sendClientPacketToServer(const Packet& packet);
	template<class Packet> void sendServerPacketToClient(Packet& packet);
	void sendServerPacketToClient(TS_SC_CHAT& packet);
	template<class Packet> void sendClientPacketToServer(Packet& packet);

	virtual EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet) override;
	void onAccountWithAuth(const TS_CS_ACCOUNT_WITH_AUTH* packet);
	void onCharacterList(const TS_CS_CHARACTER_LIST* packet);
	void onLogin(const TS_CS_LOGIN* packet);
	void onChatRequest(const TS_CS_CHAT_REQUEST* packet, bool* forwardPacket);
	void onTargeting(const TS_CS_TARGETING* packet);

private:
	~GameClientSession();

	// Current main player
	ConnectionToServer* connectionToServer = nullptr;
	// The player currently controlled
	ConnectionToServer* controlledServer = nullptr;
	// All connected players
	std::list<ConnectionToServer> connections;

	ar_handle_t localPlayerClientHandle{};
	ar_handle_t lastTargetedPlayerClientHandle{};
	bool isClientLogged = false;

	std::vector<int32_t> timeSync;
};

