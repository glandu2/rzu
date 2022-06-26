#pragma once

#include "Core/Timer.h"
#include "IFilter.h"
#include "LibGlobal.h"
#include "LuaEndpoint.h"
#include <lua.hpp>

class GlobalLuaState {
public:
	static GlobalLuaState* getInstance();

	lua_State* getLuaState();

	static int luaMessageHandler(lua_State* L);

	bool onClientConnected(int clientEndpoint, int serverEndpoint);
	bool onServerPacket(int clientEndpoint,
	                    int serverEndpoint,
	                    const TS_MESSAGE* packet,
	                    int packetVersion,
	                    SessionType sessionType,
	                    bool isStrictForwardEnabled);
	bool onClientPacket(int clientEndpoint,
	                    int serverEndpoint,
	                    const TS_MESSAGE* packet,
	                    int packetVersion,
	                    SessionType sessionType,
	                    bool isStrictForwardEnabled);
	bool onServerDisconnected(int clientEndpoint, int serverEndpoint);
	bool onClientDisconnected(int clientEndpoint, int serverEndpoint);

private:
	GlobalLuaState();
	~GlobalLuaState();

	void closeLuaVM();
	void createLuaVM();
	void initLuaVM();
	void deinitLuaVM();

	static int luaPrintToLogs(lua_State* L);

	bool luaCallPacket(int function,
	                   const char* functionName,
	                   int clientEndpoint,
	                   int serverEndpoint,
	                   const TS_MESSAGE* packet,
	                   int version,
	                   bool isServerPacket,
	                   SessionType sessionType,
	                   bool isStrictForwardEnabled);
	bool luaCallUnknownPacket(int clientEndpoint,
	                          int serverEndpoint,
	                          const TS_MESSAGE* packet,
	                          bool isServerPacket,
	                          SessionType sessionType,
	                          bool isStrictForwardEnabled);

	bool luaCallOnConnectionEvent(int luaOnEventFunction,
	                              const char* functionName,
	                              int clientEndpoint,
	                              int serverEndpoint);

	bool pushPacket(lua_State* L, const TS_MESSAGE* packet, int version, bool isServer, SessionType sessionType);

	void onCheckReload();

private:
	lua_State* L = nullptr;

	struct LuaScriptFunctionRef {
		const char* const name;
		const char* const signature;
		int ref;

		LuaScriptFunctionRef(const char* name, const char* signature)
		    : name(name), signature(signature), ref(LUA_NOREF) {}
	};

	LuaScriptFunctionRef lua_onServerPacketFunction{
	    "onServerPacket",
	    "bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* "
	    "packet, SessionType sessionType)"};
	LuaScriptFunctionRef lua_onClientPacketFunction{
	    "onClientPacket",
	    "bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* "
	    "packet, SessionType sessionType)"};
	LuaScriptFunctionRef lua_onUnknownPacketFunction{
	    "onUnknownPacket",
	    "bool onUnknownPacket(IFilterEndpoint* fromEndpoint, IFilterEndpoint* toEndpoint, int "
	    "packetId, SessionType sessionType, bool isServerPacket)"};
	LuaScriptFunctionRef lua_onClientConnectedFunction{
	    "onClientConnected", "bool onClientConnected(IFilterEndpoint* client, IFilterEndpoint* server)"};

	LuaScriptFunctionRef lua_onServerDisconnectedFunction{
	    "onServerDisconnected", "bool onServerDisconnected(IFilterEndpoint* client, IFilterEndpoint* server)"};
	LuaScriptFunctionRef lua_onClientDisconnectedFunction{
	    "onClientDisconnected", "bool onClientDisconnected(IFilterEndpoint* client, IFilterEndpoint* server)"};

	const std::vector<LuaScriptFunctionRef*> luaFunctions;

	Timer<GlobalLuaState> reloadCheckTimer;
	time_t currentLuaFileMtime;
	time_t newLuaFileMtime;
};

class PacketFilter : public IFilter {
public:
	PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, SessionType sessionType, PacketFilter* data);
	~PacketFilter();

	virtual bool onServerPacket(const TS_MESSAGE* packet) override;
	virtual bool onClientPacket(const TS_MESSAGE* packet) override;
	virtual void onServerDisconnected() override;
	virtual void onClientDisconnected() override;

private:
	int lua_clientEndpointRef = LUA_NOREF;
	int lua_serverEndpointRef = LUA_NOREF;
	LuaEndpoint* clientEndpoint;
	LuaEndpoint* serverEndpoint;
};

extern "C" {
SYMBOL_EXPORT void* initializeGlobalFilter();
SYMBOL_EXPORT void destroyGlobalFilter(void* globalData);

SYMBOL_EXPORT IFilter* createFilter(IFilterEndpoint* client,
                                    IFilterEndpoint* server,
                                    SessionType sessionType,
                                    IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}
