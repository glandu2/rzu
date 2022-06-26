#pragma once

#include "Core/Timer.h"
#include "IFilter.h"
#include <lua.hpp>

class IFilterEndpoint;

class LuaEndpoint : public Object {
	DECLARE_CLASS(LuaEndpoint)
public:
	static LuaEndpoint* createInstance(lua_State* L,
	                                   IFilterEndpoint* endpoint,
	                                   bool isServerEndpoint,
	                                   SessionType sessionType);
	void detachEndpoint();

	static void initLua(lua_State* L);
	static int index(lua_State* L);
	static int newindex(lua_State* L);
	static int gc(lua_State* L);

private:
	static const char* const NAME;
	static const luaL_Reg FUNCTIONS[];

	LuaEndpoint(IFilterEndpoint* endpoint, bool isServerEndpoint, SessionType sessionType)
	    : endpoint(endpoint), isServerEndpoint(isServerEndpoint), sessionType(sessionType) {}
	~LuaEndpoint();

	static LuaEndpoint* check_userdata(lua_State* L, int idx);

	static int getPacketVersion(lua_State* L);
	static int sendPacket(lua_State* L);
	static int close(lua_State* L);
	static int getIp(lua_State* L);
	static int banIp(lua_State* L);

	template<class T> struct SendPacketFromLuaCallback;
	template<class T> static bool sendPacketFromLua(lua_State* L, IFilterEndpoint* endpoint);

	IFilterEndpoint* endpoint;
	bool isServerEndpoint;
	SessionType sessionType;
	int userdatatable = LUA_NOREF;
};
