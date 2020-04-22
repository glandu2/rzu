#pragma once

#include "Core/Timer.h"
#include "IFilter.h"
#include "LibGlobal.h"
#include <lua.hpp>

struct LuaEndpointMetaTable {
	static const char* const NAME;
	static const luaL_Reg FUNCTIONS[];

	static int getPacketVersion(lua_State* L);
	static int sendPacket(lua_State* L);
	static int close(lua_State* L);
	static int getIp(lua_State* L);
	static int banIp(lua_State* L);

	template<class T> static bool sendPacketFromLua(lua_State* L, IFilterEndpoint* endpoint);

	IFilterEndpoint* endpoint;
	bool isServerEndpoint;
	IFilter::ServerType serverType;

	LuaEndpointMetaTable(IFilterEndpoint* endpoint, bool isServerEndpoint, IFilter::ServerType serverType)
	    : endpoint(endpoint), isServerEndpoint(isServerEndpoint), serverType(serverType) {}
};

class PacketFilter : public IFilter {
public:
	PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, ServerType serverType, PacketFilter* data);
	~PacketFilter();

	virtual bool onServerPacket(const TS_MESSAGE* packet) override;
	virtual bool onClientPacket(const TS_MESSAGE* packet) override;
	virtual void onClientDisconnected() override;

	static int luaMessageHandler(lua_State* L);

protected:
	void initLuaVM();
	void deinitLuaVM();
	static int luaPrintToLogs(lua_State* L);

	bool luaCallPacket(
	    int function, const char* functionName, const TS_MESSAGE* packet, int version, bool isServerPacket);
	bool luaCallUnknownPacket(const TS_MESSAGE* packet, bool isServerPacket);
	bool luaCallOnDisconnected(int luaOnDisconnectedFunction, const char* functionName);
	void pushEndpoint(lua_State* L, LuaEndpointMetaTable* endpoint);
	bool pushPacket(lua_State* L, const TS_MESSAGE* packet, int version, bool isServer);

	void onCheckReload();

private:
	lua_State* L = nullptr;
	int lua_onServerPacketFunction = LUA_NOREF;
	int lua_onClientPacketFunction = LUA_NOREF;
	int lua_onUnknownPacketFunction = LUA_NOREF;
	int lua_onClientDisconnectedFunction = LUA_NOREF;
	LuaEndpointMetaTable clientEndpoint;
	LuaEndpointMetaTable serverEndpoint;

	Timer<PacketFilter> reloadCheckTimer;
	time_t currentLuaFileMtime;
	time_t newLuaFileMtime;
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilterEndpoint* client,
                                    IFilterEndpoint* server,
                                    IFilter::ServerType serverType,
                                    IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

