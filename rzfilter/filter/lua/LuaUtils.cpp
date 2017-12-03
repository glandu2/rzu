#include "LuaUtils.h"
#include "Core/Utils.h"

const luaL_Reg LuaUtils::FUNCTIONS[] = {{"getTimeInMsec", &getTimeInMsec}, {NULL, NULL}};

int LuaUtils::initLua(lua_State* L) {
	luaL_newlib(L, FUNCTIONS);

	return 1;
}

int LuaUtils::getTimeInMsec(lua_State* L) {
	uint64_t time = Utils::getTimeInMsec();

	lua_pushinteger(L, time);
	return 1;
}
