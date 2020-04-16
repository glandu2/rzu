#ifndef LUAUTILS_H
#define LUAUTILS_H

#include <lua.hpp>

class LuaUtils {
public:
	static int initLua(lua_State* L);

	static int luaMessageHandler(lua_State* L);

private:
	static const luaL_Reg FUNCTIONS[];

	static int getTimeInMsec(lua_State* L);
};

#endif  // LUAUTILS_H
