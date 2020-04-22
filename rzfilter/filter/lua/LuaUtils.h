#pragma once

#include <lua.hpp>

class LuaUtils {
public:
	static int initLua(lua_State* L);

private:
	static const luaL_Reg FUNCTIONS[];

	static int getTimeInMsec(lua_State* L);
};

