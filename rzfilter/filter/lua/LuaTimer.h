#pragma once

#include "Core/Timer.h"
#include <lua.hpp>

class LuaTimer {
public:
	static int initLua(lua_State* L);

private:
	static const char* const NAME;
	static const luaL_Reg FUNCTIONS[];

	static int createTimer(lua_State* L);
	static int gc(lua_State* L);

	static LuaTimer* check_userdata(lua_State* L, int idx);

	static int start(lua_State* L);
	static int stop(lua_State* L);
	static int again(lua_State* L);
	static int setRepeat(lua_State* L);
	static int getRepeat(lua_State* L);

	void onTimerCallback();

	Timer<LuaTimer> timer;
	lua_State* L;
	int luaUserDataRef;
	int callbackLuaFunction;
};

